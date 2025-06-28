#ifndef GPS_L96_H
#define GPS_L96_H

#include "nmea_commands.h"

#include <uart.h>


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


esp_err_t gps_l96_send_command(const char *nmea_sentence);

void gps_l96_read_task(void);

#endif // GPS_L96_H