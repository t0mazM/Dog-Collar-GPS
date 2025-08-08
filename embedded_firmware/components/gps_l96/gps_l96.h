/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef GPS_L96_H
#define GPS_L96_H

#include "nmea_commands.h"
#include "gpio_expander/gpio_expander.h"
#include "../file_system_littlefs/file_system_littlefs.h" 
#include "uart.h"
#include "minmea.h"
#include "nvs_flash.h"
#include "nvs.h"

#define GPS_L96_INIT_WAIT_TIME_MS 1000 // Time to wait for GPS module to process init commands
#define NMEA_SENTENCE_BUF_SIZE 1024 

/* NVS (Non-Volatile Storage) keys for GPS tracking state */
#define NVS_NAMESPACE "dog_collar"
#define NVS_GPS_RECOVERY_STRUCT_KEY "gps_recovery"

typedef struct {
    char filename[64];  
    bool tracking_completed;
} gps_session_status_t;


/**
 * @brief Initializes the GPS L96 module.
 *
 * This function initializes the GPS module by:
 * - Initializing the UART for communication.
 * - Enabling Easy Mode, which allows the module to store the last position for faster fix on the next start.
 *
 * @return ESP_OK on success, or an error code if initialization fails.
 *
 * @note This function be called every time the device is powered on, since it loses all settings.
 */
esp_err_t gps_l96_init(void);

/**
 * @brief Extracts and processes NMEA sentences from the received buffer.
 *
 * This function scans the provided buffer for NMEA sentences, extracts them,
 * and processes each complete sentence - parses it and stores it into external flash chip.
 * Sentences should be in NMEA format- .
 *
 * @param buffer Pointer to the buffer containing raw data received from the GPS module via UART.
 * @param read_len The number of bytes in the buffer that were read from the UART.
 * @return ESP_OK on success, or an error code if processing fails.
 *
 * @note Function assumes that the buffer contains complete NMEA sentences: Starts with `$` and ends with `\r\n`
 */

/**
 * @brief Checks if the geo-fence has been triggered.
 *
 * This function checks the state of the geo-fence pin from L96 GPS module.
 *
 * @return true if the geo-fence is triggered, false otherwise.
 */
bool gps_l96_is_geo_fence_triggered(void);

/**
 * @brief Receives the buffer with many NMEA sentences and processes them.
 * 
 * This function reads the buffer received from the GPS module via UART,
 * extracts complete NMEA sentences, and processes each sentence at a time.
 *
 * @note If more of the same NMEA sentence is received, the last one will be saved.
 * @param buffer Pointer to the buffer containing raw data received from the GPS module via UART.
 * @param read_len The number of bytes in the buffer that were read from the UART.
 */
esp_err_t gps_l96_extract_and_process_nmea_sentences(const uint8_t *buffer, size_t read_len);

/**
 * @brief Sets the GPS module to standby mode.
 *
 * This function sends the command to put the GPS module into standby mode - waiting for a command, not recording.
 * It also sets the FORCE_ON pin to high, in case the module is in deep sleep mode.
 *
 * @return ESP_OK on success, or an error code if the command fails.
 */
esp_err_t gps_l96_go_to_standby_mode(void);

/**
 * @brief Starts recording GPS data.
 *
 * This function sends the command to start recording GPS data at a rate of 1Hz and to only send RMC sentences.
 * It also sets the FORCE_ON pin to high, in case the module is in deep sleep mode.
 *
 * @note We cannot check if it was successful, because the GPS module does not respond to any commands after we send it in deep sleep mode.
 * @return ESP_OK on success, or an error code if the command fails.
 */
esp_err_t gps_l96_start_recording(void);

/**
 * @brief Puts the GPS module into back-up mode - deep sleep mode.
 *
 * This function sends the command to put the GPS module into back-up mode, which is a low-power state.
 * During this mode, the GPS module will draw 7 μA.
 * 
 * @note To wake up the GPS module, call gps_l96_go_to_standby_mode() or gps_l96_start_recording()
 * @return ESP_OK on success, or an error code if the command fails.
 */
esp_err_t gps_l96_go_to_back_up_mode(void);


/**
 * @brief Sends a command to the GPS module.
 *
 * This function sends a NMEA sentence to the GPS module via UART.
 * The sentence should be in NMEA format: start with `$` and end with `\r\n`.
 *
 * @param nmea_sentence Pointer to the NMEA sentence to be sent (can be found in nmea_commands.h).
 * @return ESP_OK on success, or an error code if sending fails.
 */
esp_err_t gps_l96_send_command(const char *nmea_sentence);

/**
 * @brief Extracts-parses GPS data from a NMEA sentence.
 *
 * This function extracts-parses GPS data from NMEA sentence and stores struct gps_rcm_data.
 *
 * @param nmea_sentence Pointer to the NMEA sentence to be processed.
 * @return ESP_OK on success, or an error code if extraction fails.
 */
esp_err_t gps_l96_extract_data_from_nmea_sentence(const char *nmea_sentence);


void gps_l96_print_data(void);

/**
 * @brief Formats a CSV line from the GPS data.
 *
 * This function formats data from the global struct gps_rcm_data into a CSV line that can be appended to a file.
 *
 * @param file_line Pointer to the buffer where the formatted CSV line will be stored.
 * @param file_line_size Size of the buffer.
 * @return ESP_OK on success, or an error code if formatting fails.
 */
esp_err_t gps_l96_format_csv_line_from_data(char *file_line, size_t file_line_size);

/**
 * @brief Checks if the GPS module has a valid fix.
 *
 * This function checks if the GPS module has a valid fix based on the validity of the GPS data.
 *
 * @return true if the GPS module has a valid fix, false otherwise.
 */
bool gps_l96_has_fix(void);

/**
 * @brief Gets the date string from the GPS data.
 *
 * This function formats the date from the global struct gps_rcm_data into a string in the format "YYYY-MM-DD".
 *
 * @param date_string Pointer to the buffer where the formatted date string will be stored.
 * @param date_string_size Size of the buffer.
 * @return ESP_OK on success, or an error code if formatting fails.
 */
esp_err_t gps_l96_get_date_string_from_data(char *date_string, size_t date_string_size);

void gpio_reset_gps(void);

/**
 * @brief Sets the FORCE_ON pin state for the GPS module.
 * 
 * This function sets the FORCE_ON pin to high or low to wake up the GPS module from deep sleep mode/backup mode.
 * It is good practice to set it to high before sending any commands to the GPS module.
 * 
 * @param enable true to set FORCE_ON pin high, false to set it low.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t gps_force_on_set(bool enable);

/**
 * @brief Start GPS activity tracking with specified filename
 * 
 * @param filename Name of the GPS file being created
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t gps_l96_start_activity_tracking(const char *filename);

/**
 * @brief Stop GPS activity tracking normally
 * 
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t gps_l96_stop_activity_tracking(void);

/**
 * @brief Check if GPS tracking recovery is needed and get interrupted filename
 * 
 * @param filename Buffer to store the interrupted filename
 * @param filename_size Size of the filename buffer
 * @param recovery_needed Pointer to store the recovery status
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t gps_check_recovery_needed(char *filename, size_t filename_size, bool *recovery_needed);




#endif // GPS_L96_H