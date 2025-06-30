#include "gps_l96.h"

static const char *TAG = "GPS_L96";

gps_state_t gps_state = GPS_STATE_IDLE; // Default state on power on

esp_err_t gps_l96_init(void) {
    gps_l96_send_command(GNSS_MODE_GPS_GLONASS);
    gps_state = GPS_STATE_IDLE;
    return ESP_OK;
}

esp_err_t gps_l96_go_to_standby_mode(void) {

    // Set FORCE_ON pin to high (in case we are in deep sleep mode)
    ESP_RETURN_ON_ERROR(gpio_set_pin_force(true), TAG, "Failed to set FORCE_ON pin"); 
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GPS_STAND_BY_MODE), TAG, "Failed to send GPS_STAND_BY_MODE command");
    gps_state = GPS_STATE_IDLE;
    return ESP_OK;
}

esp_err_t gps_l96_start_recording(void) {

    gpio_reset_gps();
    gpio_set_pin_force(false);

    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_MODE_GPS_GLONASS), TAG, "Failed to send GNSS_MODE_GPS_GLONASS command");
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_SET_UPDATE_RATE_1HZ), TAG, "Failed to send GNSS_SET_UPDATE_RATE_1HZ command");
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_QUERY_UPDATE_RATE), TAG, "Failed to send GNSS_QUERY_UPDATE_RATE command");
    gps_state = GPS_STATE_RECORDING;
    return ESP_OK;
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

    char NMEA_sentence[NMEA_SENTENCE_BUF_SIZE]; // Buffer to hold the sentence/command
    int sentence_idx = 0;
    bool reading_sentence = false; // Flag to indicate if we are currently reading a sentence

    // Check if buffer is ok and it is not empty
    if (buffer == NULL || read_len == 0) {
        ESP_LOGE(TAG, "Invalid buffer or read length");
        return ESP_ERR_INVALID_ARG;
    }
    // Loop through each byte in the buffer
    for (size_t buf_idx = 0; buf_idx < read_len; buf_idx++) {
        char ch = buffer[buf_idx]; 

        // 1. Check for $ and
        if(ch == '$'){
            reading_sentence = true; 
            sentence_idx = 0; 
        }

        if(reading_sentence) {

        // 2. Add character to the sentence buffer
        if (sentence_idx < (int)sizeof(NMEA_sentence) - 1) { //- only if sentence buffer is not full
            NMEA_sentence[sentence_idx++] = ch;
        } else { //Buffer if full
            ESP_LOGW(TAG, "NMEA sentence buffer overflow.");
            sentence_idx = 0; 
        }

        // 3. Check for end of sentence "\n" or "\r\n"
        if (ch == '\n' && sentence_idx >= 2 && NMEA_sentence[sentence_idx - 2] == '\r') {
            NMEA_sentence[sentence_idx] = '\0';

            sentence_idx = 0; 
            reading_sentence = false; 

            // 3. Process the complete NMEA sentence TODO
            ESP_LOGI(TAG, "L96 Response: %s", NMEA_sentence);
            gps_l96_extract_data_from_nmea_sentence(NMEA_sentence, NULL); 
        }
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

esp_err_t gps_l96_extract_data_from_nmea_sentence(const char *nmea_sentence, gps_l96_data_t *gps_data) {


    struct minmea_sentence_rmc frame;
    if (minmea_check(nmea_sentence, false)) {
        printf( "NMEA ID: %d \n",  minmea_sentence_id(nmea_sentence, false)  );
    }
    if (minmea_sentence_id(nmea_sentence, false) == MINMEA_SENTENCE_RMC) {
        // parse RMC
    } else if (minmea_sentence_id(nmea_sentence, false) == MINMEA_SENTENCE_VTG) {
        // parse VTG
    }

    if (minmea_parse_rmc(&frame, nmea_sentence)) {
        printf("Time: %02d:%02d:%02d\n", frame.time.hours, frame.time.minutes, frame.time.seconds);
        printf("Validity: %c\n", frame.valid ? 'A' : 'V');
        printf("Latitude: %f\n", minmea_tocoord(&frame.latitude));
        printf("Longitude: %f\n", minmea_tocoord(&frame.longitude));
        printf("Speed (knots): %f\n", minmea_tofloat(&frame.speed));
        printf("Date: %02d/%02d/%04d\n", frame.date.day, frame.date.month, frame.date.year + 2000);
    } else {
        printf("Failed to parse RMC sentence.\n");
    }
    return ESP_OK;

}
