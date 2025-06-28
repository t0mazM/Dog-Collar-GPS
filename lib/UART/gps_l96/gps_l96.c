#include "gps_l96.h"

static const char *TAG = "GPS_L96";

esp_err_t gps_l96_init(void) {
    gps_l96_send_command(GNSS_SET_UPDATE_RATE_1HZ); 
    gps_l96_send_command(GNSS_MODE_GPS_GLONASS); 

    return uart_init();
}

esp_err_t gps_l96_send_command(const char *nmea_sentence) {
    if (nmea_sentence == NULL || strlen(nmea_sentence) == 0) {
        ESP_LOGE(TAG, "Invalid NMEA sentence");
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Sending NMEA command: %s", nmea_sentence);
    return uart_send_cmd(nmea_sentence, strlen(nmea_sentence));
}



esp_err_t gps_l96_extract_and_process_nmea_sentences(const uint8_t *buffer, size_t read_len) {

    char NMEA_sentence[255]; // Buffer to hold the sentence/command
    int sentence_idx = 0;

    // Check if buffer is ok and it is not empty
    if (buffer == NULL || read_len == 0) {
        ESP_LOGE(TAG, "Invalid buffer or read length");
        return ESP_ERR_INVALID_ARG;
    }
    // Loop through each byte in the buffer
    for (size_t buf_idx = 0; buf_idx < read_len; buf_idx++) {
        char ch = buffer[buf_idx]; 

        // 1. Look for the start of a NMEA sentence
        if (ch == '$') {
            sentence_idx = 0;
            NMEA_sentence[sentence_idx++] = '$';
        }

        // 2. Add character to the sentence buffer - only if sentence buffer is not full
        if (sentence_idx < (int)sizeof(NMEA_sentence) - 1) {
            NMEA_sentence[sentence_idx++] = ch;
        } else { //Buffer if full
            ESP_LOGW(TAG, "NMEA sentence buffer overflow.");
            sentence_idx = 0; 
        }

        // 3. Check for end of sentence "\n" or "\r\n"
        if (ch == '\n' && sentence_idx >= 2 && NMEA_sentence[sentence_idx - 2] == '\r') {
            NMEA_sentence[sentence_idx] = '\0';
            sentence_idx = 0; 
            // 4. Process the complete NMEA sentence TODO
            ESP_LOGI(TAG, "L96 Response: %s", NMEA_sentence);
            
        }
    }
    return ESP_OK;
    
}

void gps_l96_read_task(void) { //Just a dummy task to test the GPS module
    uint8_t rx_buffer[255];
    size_t read_len = 0;

    printf("-------- UART READ ------------\n");
    esp_err_t err = uart_receive_cmd(rx_buffer, sizeof(rx_buffer), &read_len);
    if (err == ESP_OK && read_len > 0) {
        gps_l96_extract_and_process_nmea_sentences(rx_buffer, read_len);
    }
}