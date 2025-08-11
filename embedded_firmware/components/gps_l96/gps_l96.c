/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "gps_l96.h"
static const char *TAG = "GPS_L96";

struct minmea_sentence_rmc gps_rcm_data; // GPS RMC data structure to hold parsed data

static esp_err_t gps_nvs_save_session_status(char* filename, size_t filename_size, bool completed_normally);
static esp_err_t gps_nvs_load_session_status(char* filename, size_t filename_size, bool *completed_normally);


esp_err_t gps_l96_init(void) {

    ESP_RETURN_ON_ERROR(uart_init(), 
                        TAG, "Failed to initialize UART for GPS L96");

    gpio_reset_gps(); 

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

    gps_l96_start_recording();

    ESP_RETURN_ON_ERROR(nvs_flash_init(), 
                        TAG, "Failed to initialize NVS flash");

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
    //note: we can't check if if was send succesfull because gps modeule goes into deep sleep and it does not respond to any commands
    gps_l96_send_command(GPS_DEEP_SLEEP_MODE); 
    
    return ESP_OK;
}

esp_err_t gps_l96_send_command(const char *nmea_sentence) {
    if (nmea_sentence == NULL || strlen(nmea_sentence) == 0) {
        ESP_LOGE(TAG, "Invalid NMEA sentence");
        return ESP_ERR_INVALID_ARG;
    }
    // ESP_LOGI(TAG, "Sending NMEA command: %s", nmea_sentence);
    return uart_send_cmd(nmea_sentence, strlen(nmea_sentence));
}

void gpio_reset_gps(void) {
    uint8_t state = gpio_expander_get_output_state();
    state &= ~GPS_RESET;
    gpio_expander_update_output_state(state);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    state |= GPS_RESET;
    gpio_expander_update_output_state(state);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "GPS reset done");
}

esp_err_t gps_force_on_set(bool enable){
    uint8_t state = gpio_expander_get_output_state();
    uint8_t new_state = enable
                        ? (state |  GPS_FORCE_ON)
                        : (state & ~GPS_FORCE_ON);

    if (new_state == state) {
        return ESP_OK;
    }

    gpio_expander_update_output_state(new_state);
    return ESP_OK;
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

                // 4. Process the complete NMEA sentence TODO
                // ESP_LOGI(TAG, "L96 Response: %s", NMEA_sentence);
                gps_l96_extract_data_from_nmea_sentence(NMEA_sentence); 
            }
        }
    }
    return ESP_OK;
    
}

esp_err_t gps_l96_extract_data_from_nmea_sentence(const char *nmea_sentence) {

    enum minmea_sentence_id nmea_id = minmea_sentence_id(nmea_sentence, false);

    switch(nmea_id) {
        case MINMEA_SENTENCE_RMC:
            minmea_parse_rmc(&gps_rcm_data, nmea_sentence);
            gps_rcm_data.date.year += 2000; // by default GPS module returns year as 0-99, so we add 2000 for correct year
            //gps_l96_print_data();
            break;
        case MINMEA_SENTENCE_VTG:
            /*TODO: add parsing for all other sentences types
            Currently I need only RMC sentence*/
            break;
        default:
            ESP_LOGW(TAG, "Unknown NMEA sentence ID: %d", nmea_id);
            break; // Ignore sentences with errors
    }
    return ESP_OK;
}

void gps_l96_print_data(void){ // a DEBUG function to print GPS data
            printf("Time: %02d:%02d:%02d\n", gps_rcm_data.time.hours, gps_rcm_data.time.minutes, gps_rcm_data.time.seconds);
            printf("Validity: %c\n", gps_rcm_data.valid ? 'A' : 'V');
            printf("Latitude: %f\n", minmea_tocoord(&gps_rcm_data.latitude));
            printf("Longitude: %f\n", minmea_tocoord(&gps_rcm_data.longitude));
            printf("Speed (knots): %f\n", minmea_tofloat(&gps_rcm_data.speed));
            printf("Date: %02d/%02d/%04d\n", gps_rcm_data.date.day, gps_rcm_data.date.month, gps_rcm_data.date.year);

}

esp_err_t gps_l96_start_activity_tracking(const char *filename) {  // Make const since we're reading from it
    // Start recording
    ESP_RETURN_ON_ERROR(gps_l96_start_recording(), 
                        TAG, 
                        "Failed to start GPS recording");
        
    /* Save session state */
    /* Set to false, so if we crash, we can see if we need to recover */
    ESP_RETURN_ON_ERROR(gps_nvs_save_session_status((char*)filename, strlen(filename), false), 
                        TAG,
                        "Failed to save GPS session completion state");
    return ESP_OK;
}

esp_err_t gps_l96_stop_activity_tracking(void) {  // Remove filename parameter - we don't need it
    // Stop recording
    esp_err_t ret = gps_l96_go_to_standby_mode(); 
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop GPS recording");
    }
    // Mark session as completed - clear filename since session is done
    ESP_RETURN_ON_ERROR(gps_nvs_save_session_status("", 0, true), 
                        TAG, 
                        "Failed to save GPS session completion state");
    return ESP_OK;
}

esp_err_t gps_check_recovery_needed(char *filename, size_t filename_size, bool *recovery_needed) {  // Add missing parameter
    bool session_completed_normally = true;

    ESP_RETURN_ON_ERROR(gps_nvs_load_session_status(filename, filename_size, &session_completed_normally),
                        TAG, "Failed to load GPS session status");

    *recovery_needed = !session_completed_normally;
        
    return ESP_OK;
}

static esp_err_t gps_nvs_save_session_status(char* filename, size_t filename_size, bool completed_normally) {
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    gps_session_status_t gps_session_status = {0}; 


    /* Add filename to struct*/
    if (filename != NULL) {
        strncpy(gps_session_status.filename, filename, sizeof(gps_session_status.filename) - 1);
        gps_session_status.filename[sizeof(gps_session_status.filename) - 1] = '\0';
    }
    /* Add completion flag to struct */
    gps_session_status.tracking_completed = completed_normally;

    /* Open NVS */
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle for writing: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Save struct to NVS */
    ret = nvs_set_blob(nvs_handle, NVS_GPS_RECOVERY_STRUCT_KEY, &gps_session_status, sizeof(gps_session_status));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save GPS session status struct to NVS: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    /* Commit changes */
    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit NVS changes: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Saved GPS session status struct: file='%s', completed=%s",
             gps_session_status.filename,
             gps_session_status.tracking_completed ? "true" : "false");

    nvs_close(nvs_handle);
    return ESP_OK;
}

static esp_err_t gps_nvs_load_session_status(char *filename, size_t filename_size, bool *completed_normally) {
    gps_session_status_t gps_session_status = {
        .tracking_completed = true,
        .filename = "default_file_name"
    }; 
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    size_t required_size = sizeof(gps_session_status_t);

    /* Open NVS */
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    
    // Handle namespace not found (first boot)
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        gps_nvs_save_session_status(gps_session_status.filename, 
                                    sizeof(gps_session_status.filename), 
                                    gps_session_status.tracking_completed);
        ESP_LOGE(TAG, "No nvs data found. Making new entry with filename='%s' and completed=%s",
                 gps_session_status.filename,
                 gps_session_status.tracking_completed ? "true" : "false"); 
    }

    /* again open NVS */
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS handle for reading: %s", esp_err_to_name(ret));
        *completed_normally = true; // Set safe default
        return ret;
    }

    ret = nvs_get_blob(nvs_handle, NVS_GPS_RECOVERY_STRUCT_KEY, &gps_session_status, &required_size);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "GPS session state key not found in NVS (first boot)");
        *completed_normally = true; // Set safe default
        nvs_close(nvs_handle);
        return ESP_OK; // Not an error, just first boot
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read GPS session state from NVS: %s", esp_err_to_name(ret));
        *completed_normally = true; // Set safe default
        nvs_close(nvs_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Loaded GPS session state: file='%s', completed=%s",
            gps_session_status.filename,
            gps_session_status.tracking_completed ? "true" : "false");
    
    /* Update output parameters */
    if (filename != NULL && filename_size > 0) {
        strncpy(filename, gps_session_status.filename, filename_size - 1);
        filename[filename_size - 1] = '\0';
    }
    *completed_normally = gps_session_status.tracking_completed;

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t gps_l96_format_csv_line_from_data(char *file_line, size_t file_line_size) {

    // Format: ISO 8601 timestamp,latitude,longitude,altitude,speed
    int written = snprintf(file_line, file_line_size,
        "%04d-%02d-%02dT%02d:%02d:%02dZ,%f,%f,%f,%f\n",
        gps_rcm_data.date.year,
        gps_rcm_data.date.month,
        gps_rcm_data.date.day,
        gps_rcm_data.time.hours,
        gps_rcm_data.time.minutes,
        gps_rcm_data.time.seconds,
        minmea_tocoord(&gps_rcm_data.latitude),
        minmea_tocoord(&gps_rcm_data.longitude),
        minmea_tofloat(&gps_rcm_data.speed),
        0.0);

    if (written < 0 || (size_t)written >= file_line_size) {
        ESP_LOGE(TAG, "Failed to format GPS data into file line");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

bool gps_l96_has_fix(void) {
    return gps_rcm_data.valid;
}

esp_err_t gps_l96_get_date_string_from_data(char *date_string, size_t date_string_size) {

    int written = snprintf(date_string, date_string_size, "%04d-%02d-%02d",
                           gps_rcm_data.date.year,
                           gps_rcm_data.date.month,
                           gps_rcm_data.date.day);

    if (written < 0 || (size_t)written >= date_string_size) {
        ESP_LOGE(TAG, "Failed to format date string");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}