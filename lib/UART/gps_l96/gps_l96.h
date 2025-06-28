#ifndef GPS_L96_H
#define GPS_L96_H

#include "nmea_commands.h"
#include "gpio_extender/gpio_extender.h"
#include <uart.h>

typedef enum {
    GPS_STATE_IDLE,
    GPS_STATE_WAIT_FIX,
    GPS_STATE_HAS_FIX,
    GPS_STATE_RECORDING,
    GPS_STATE_DEEP_SLEEP
} gps_state_t;



/**
 * @brief Initializes the GPS L96 module.
 *
 * This function initializes the GPS module by sending the NMEA commands.
 * All settings are lost after power off, so the initialization must be
 * called every time the device is powered on.
 *
 * @return ESP_OK on success, or an error code if initialization fails.
 *
 * @note This function be called every time the device is powered on.
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
 * This function sends the command to start recording GPS data at a rate of 1Hz.
 *
 * @return ESP_OK on success, or an error code if the command fails.
 */
esp_err_t gps_l96_start_recording(void);


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
 * @brief Task to read data from the GPS module.
 *
 * This function reads data from the GPS module via UART and processes the received NMEA sentences.
 * It should be called periodically to ensure that the GPS data is read and processed.
 * Currently, it is a dummy task to test the GPS module.
 */
void gps_l96_read_task(void);

#endif // GPS_L96_H