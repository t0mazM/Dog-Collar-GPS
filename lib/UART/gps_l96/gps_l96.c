#include "gps_l96.h"


esp_err_t parse_uart_data(const uint8_t *buffer, size_t read_len) {
    printf("-------------------------PARSING---------------------------\n");
    char NMEA_sentence[255]; // Buffer to hold the sentence/command
    int sentence_idx = 0;

    for (int i = 0; i < read_len; i++) {
        char ch = buffer[i];

        if (ch == '$') {
            sentence_idx = 0; // Start new sentence
        }

        if (sentence_idx < (int)sizeof(NMEA_sentence) - 1) {
            NMEA_sentence[sentence_idx++] = ch;
        }

        if (ch == '\n' && sentence_idx >= 2 && NMEA_sentence[sentence_idx - 2] == '\r') {
            NMEA_sentence[sentence_idx] = '\0';
            ESP_LOGI("main", "L96 Response: %s", NMEA_sentence);
            sentence_idx = 0; // Reset for next sentence
        }
    }
    return ESP_OK;
    
}