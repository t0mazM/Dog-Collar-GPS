#ifndef GPS_L96_H
#define GPS_L96_H


#include <uart.h>

esp_err_t gps_l96_extract_and_process_nmea_sentences(const uint8_t *buffer, size_t read_len);

#endif // GPS_L96_H