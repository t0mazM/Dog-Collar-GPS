#include "gps_l96.h"

static const char *TAG = "GPS_L96";


struct minmea_sentence_rmc gps_rcm_data; // GPS RMC data structure to hold parsed data

esp_err_t gps_l96_init(void) {

    ESP_RETURN_ON_ERROR(uart_init(), 
                        TAG, "Failed to initialize UART for GPS L96");

    /*Delay so that GPS module can initialize properly - MUST have.*/ 
    vTaskDelay(pdMS_TO_TICKS(GPS_L96_INIT_WAIT_TIME_MS));

    /*Enable Easy Mode - it stores the last position so on next gps start it gets fix faster*/
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_ENABLE_EASY), 
                        TAG, 
                        "Failed to send GNSS_ENABLE_EASY command");

    vTaskDelay(pdMS_TO_TICKS(GPS_L96_INIT_WAIT_TIME_MS));

    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_SET_UPDATE_RATE_1HZ), 
                        TAG, 
                        "Failed to send GNSS_SET_UPDATE_RATE_1HZ command");

    vTaskDelay(pdMS_TO_TICKS(GPS_L96_INIT_WAIT_TIME_MS)); //TODO: check if this is needed

    ESP_RETURN_ON_ERROR(gps_l96_send_command(ONLY_GNRMC), 
                        TAG, 
                        "Failed to send ONLY_GNRMC command");
    return ESP_OK;
}

bool gps_l96_is_geo_fence_triggered(void) {
    uint8_t input_state = 0;

    gpio_read_inputs(&input_state);
    return (input_state & GEO_FENCE) != 0;
}

esp_err_t gps_l96_go_to_standby_mode(void) {

    ESP_RETURN_ON_ERROR(gps_force_on_set(true), TAG, "Failed to set FORCE_ON pin");  // Set FORCE_ON pin to high (in case we are in deep sleep mode)
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GPS_STAND_BY_MODE), TAG, "Failed to send GPS_STAND_BY_MODE command");
    return ESP_OK;
}

esp_err_t gps_l96_start_recording(void) {

    gpio_reset_gps();
    gps_force_on_set(true); //Crucial to set it to HIGH

    
    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_MODE_GPS_GLONASS), 
                        TAG, 
                        "Failed to send GNSS_MODE_GPS_GLONASS command");

    vTaskDelay(1000 / portTICK_PERIOD_MS); // TODO: check if this is needed

    ESP_RETURN_ON_ERROR(gps_l96_send_command(GNSS_SET_UPDATE_RATE_1HZ), 
                        TAG, 
                        "Failed to send GNSS_SET_UPDATE_RATE_1HZ command");
   
    vTaskDelay(1000 / portTICK_PERIOD_MS); // TODO: check if this is needed

    ESP_RETURN_ON_ERROR(gps_l96_send_command(ONLY_GNRMC), 
                        TAG, 
                        "Failed to send ONLY_GNRMC command");
    return ESP_OK;
}

esp_err_t gps_l96_go_to_back_up_mode(void) { // same as deep sleep mode
    gps_force_on_set(false); 
    //note: we can't check if if was send succesfull becouse gps modeule goes into ddepsleep and it does not respond to any commands
    gps_l96_send_command(GPS_DEEP_SLEEP_MODE); 
    
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
                reading_sentence = false; 
            }

            // 3. Check for end of sentence "\n" or "\r\n"
            if (ch == '\n' && sentence_idx >= 2 && NMEA_sentence[sentence_idx - 2] == '\r') {
                NMEA_sentence[sentence_idx] = '\0';

                sentence_idx = 0; 
                reading_sentence = false; 

                // 3. Process the complete NMEA sentence TODO
                ESP_LOGI(TAG, "L96 Response: %s", NMEA_sentence);
                gps_l96_extract_data_from_nmea_sentence(NMEA_sentence); 
            }
        }
    }
    return ESP_OK;
    
}

void gps_l96_read_task(void) { //Just a dummy task to test the GPS module
    uint8_t rx_buffer[UART_RX_BUF_SIZE];
    memset(rx_buffer, 0, sizeof(rx_buffer)); // Clear the buffer
    size_t read_len = 0;

    printf("-------- UART READ ------------\n");
    esp_err_t err = uart_receive_cmd(rx_buffer, sizeof(rx_buffer), &read_len);
    if (err == ESP_OK && read_len > 0) {
        gps_l96_extract_and_process_nmea_sentences(rx_buffer, read_len);
    }
}

esp_err_t gps_l96_extract_data_from_nmea_sentence(const char *nmea_sentence) {

    
    enum minmea_sentence_id nmea_id = minmea_sentence_id(nmea_sentence, false);

    switch(nmea_id) {
        case MINMEA_SENTENCE_RMC:
            minmea_parse_rmc(&gps_rcm_data, nmea_sentence);
            gps_l96_print_data();
            break;
        case MINMEA_SENTENCE_VTG:
            /*TODO: add parsing for all other sentences types
            Currently I need only RMC sentence*/
            break;
        default:
            ESP_LOGW(TAG, "Unknown NMEA sentence ID: %d", nmea_id);
            return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

void gps_l96_print_data(void){ // a DEBUG function to print GPS data
            printf("Time: %02d:%02d:%02d\n", gps_rcm_data.time.hours, gps_rcm_data.time.minutes, gps_rcm_data.time.seconds);
            printf("Validity: %c\n", gps_rcm_data.valid ? 'A' : 'V');
            printf("Latitude: %f\n", minmea_tocoord(&gps_rcm_data.latitude));
            printf("Longitude: %f\n", minmea_tocoord(&gps_rcm_data.longitude));
            printf("Speed (knots): %f\n", minmea_tofloat(&gps_rcm_data.speed));
            printf("Date: %02d/%02d/%04d\n", gps_rcm_data.date.day, gps_rcm_data.date.month, gps_rcm_data.date.year + 2000);

}

esp_err_t gps_l96_start_activity_tracking(void) {

    // Start recording
    ESP_RETURN_ON_ERROR(gps_l96_start_recording(), 
                        TAG, 
                        "Failed to start GPS recording");


    return ESP_OK;
}