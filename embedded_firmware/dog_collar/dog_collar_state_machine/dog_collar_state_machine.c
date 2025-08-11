/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "dog_collar_state_machine.h"

static const char *TAG = "DOG_COLLAR_STATE_MACHINE";

/* Function Prototypes */
static char *get_current_state_string(dog_collar_state_t state);
static esp_err_t gps_tracking_task(char *gps_file_name);

/* Global variables for dog collar state machine */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;
static dog_collar_state_t previous_state = DOG_COLLAR_STATE_INITIALIZING;
static uint32_t state_entry_time = 0;
static char gps_file_name[LFS_MAX_FILE_NAME_SIZE] = {0};
static bool gps_recovery_needed = false; // Used to continue GPS activity if tracking is interrupted
static bool checked_gps_recovery = false;// Indicates if GPS recovery has been checked

void state_machine_task(void *pvParameters) {
    while (true) {
        dog_collar_state_machine_run();
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

dog_collar_state_t dog_collar_state_machine_run(void) {

    // Track state entry time
    if (current_state != previous_state) {
        state_entry_time = xTaskGetTickCount();
        ESP_LOGI(TAG, "State changed from %s to %s", 
                 get_current_state_string(previous_state),
                 get_current_state_string(current_state));
        previous_state = current_state;
    }

    switch (current_state) {
        case DOG_COLLAR_STATE_INITIALIZING:
            current_state = handle_initializing_state();
            break;
        case DOG_COLLAR_STATE_NORMAL:
            current_state = handle_normal_state();
            break;
        case DOG_COLLAR_STATE_LOW_BATTERY:
            current_state = handle_low_battery_state();
            break;
        case DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY:
            current_state = handle_critical_low_battery_state();
            break;
        case DOG_COLLAR_STATE_CHARGING:
            current_state = handle_charging_state();
            break;
        case DOG_COLLAR_STATE_GPS_ACQUIRING:
            current_state = handle_gps_acquiring_state();
            break;
        case DOG_COLLAR_STATE_GPS_READY:
            current_state = handle_gps_ready_state();
            break;
        case DOG_COLLAR_STATE_WAITING_FOR_GPS_FIX:
            current_state = handle_waiting_for_gps_fix_state();
            break;
        case DOG_COLLAR_STATE_GPS_FILE_CREATION:
            current_state = handle_gps_file_creation_state();
            break;
        case DOG_COLLAR_STATE_GPS_TRACKING:
            current_state = handle_gps_tracking_state();
            break;
        case DOG_COLLAR_STATE_GPS_PAUSED:
            current_state = handle_gps_paused_state();
            break;
        case DOG_COLLAR_STATE_WIFI_SYNC:
            current_state = handle_wifi_sync_state();
            break;
        case DOG_COLLAR_STATE_LIGHT_SLEEP:
            current_state = handle_light_sleep_state();
            break;
        case DOG_COLLAR_STATE_DEEP_SLEEP:
            current_state = handle_deep_sleep_state();
            break;
        case DOG_COLLAR_STATE_ERROR:
            current_state = handle_error_state();
            break;
        default:
            // Handle unexpected state
            current_state = DOG_COLLAR_STATE_ERROR;
    }

    printf("Current state: %s\n", get_current_state_string(current_state));
    current_state = battery_management_routine(current_state);
    led_management_set_pattern(current_state);

    return current_state;
}

dog_collar_state_t battery_management_routine(dog_collar_state_t current_state) {

    static uint32_t last_battery_check = 0;
    static uint32_t battery_check_interval = 0;
    uint32_t now = xTaskGetTickCount();

    // 1) Check if it's time to check the battery
    if( (now - last_battery_check) < battery_check_interval / portTICK_PERIOD_MS) {
        return current_state; // Not time to check battery yet
    }

    // 2) Update battery data
    esp_err_t ret = battery_monitor_update_battery_data();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update battery data");
        return DOG_COLLAR_STATE_ERROR;
    }

    // 3) Update next battery check time interval
    if (battery_data.soc >= BATTERY_SOC_HIGH) {
        battery_check_interval = BATTERY_CHECK_INTERVAL_MS_HIGH;
    } else{
        battery_check_interval = BATTERY_CHECK_INTERVAL_MS_LOW;
    }

    // 4) Return correct state based on battery data:
    // a) CHARGING
    if (battery_data.current > 0) {
        ESP_LOGI(TAG, "Battery is charging");
        return DOG_COLLAR_STATE_CHARGING;
    }

    // b) CRITICAL (battery is about to be empty)
    if (battery_data.soc >= 0.0f && battery_data.soc <= 10.0f && current_state != DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY) {
        ESP_LOGW(TAG, "Battery is critically low");
        return DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY;
    }
    // c) LOW (battery is low, but not critical)
    if (battery_data.soc > 10.0f && battery_data.soc <= 20.0f && current_state != DOG_COLLAR_STATE_LOW_BATTERY) {
        ESP_LOGW(TAG, "Battery is low");
        return DOG_COLLAR_STATE_LOW_BATTERY;
    }
    // d) NORMAL If battery is OK, stay in the current state (do not go to NORMAL)
    return current_state;
}

dog_collar_state_t handle_initializing_state(void) {
   
    esp_err_t init_result = dog_collar_components_init();

    if (init_result != ESP_OK) {
        return DOG_COLLAR_STATE_ERROR;
    }

    return DOG_COLLAR_STATE_NORMAL; 
}

dog_collar_state_t handle_normal_state(void) {

    if (was_woken_by_button_press()) {
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }

    if(checked_gps_recovery == false) {
        checked_gps_recovery = true;

        gps_check_recovery_needed(gps_file_name, sizeof(gps_file_name), &gps_recovery_needed);

        if (gps_recovery_needed) {
            gps_l96_start_activity_tracking(gps_file_name);
            return DOG_COLLAR_STATE_GPS_TRACKING;
        }
    }

    // 1) Check if button was pressed
    if (is_button_short_pressed()) {
        vTaskDelay(pdMS_TO_TICKS(WAIT_AFTER_USER_PRESS_MS)); // Wait a bit and clear the button state
        clear_button_press_states();
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }
    
    // 2.) Go to WIFI_SYNC state
    return DOG_COLLAR_STATE_WIFI_SYNC;
    
}

dog_collar_state_t handle_low_battery_state(void) {

    return DOG_COLLAR_STATE_LOW_BATTERY;
}

dog_collar_state_t handle_critical_low_battery_state(void) {

    return DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY;
}

dog_collar_state_t handle_charging_state(void) {

    wifi_manager_reconnect(); 

    if(battery_data.current < 0) {
        return DOG_COLLAR_STATE_NORMAL; // If battery is not charging, go back to normal state
    }

    return DOG_COLLAR_STATE_CHARGING; 
}

dog_collar_state_t handle_gps_acquiring_state(void) {

    /* Check if time is up */
    if (xTaskGetTickCount() - state_entry_time > (GPS_ACQUIRE_TIMEOUT_MS / portTICK_PERIOD_MS) ) {
        return DOG_COLLAR_STATE_NORMAL;
    }

    /* Check for user input */
    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_WAITING_FOR_GPS_FIX; 
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            
    }

    /* Check for GPS fix */
    gps_tracking_task(NULL); 
    if(gps_l96_has_fix() ) {
        return DOG_COLLAR_STATE_GPS_READY;
    }

    return DOG_COLLAR_STATE_LIGHT_SLEEP; 
}

dog_collar_state_t handle_gps_ready_state(void) {

    /* Check for user input */
    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_FILE_CREATION; 
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            // go back to normal state
    }

    return DOG_COLLAR_STATE_LIGHT_SLEEP;
}

dog_collar_state_t handle_gps_file_creation_state(void) {

    esp_err_t ret = lfs_create_new_csv_file(gps_file_name, sizeof(gps_file_name));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create GPS file");
        return DOG_COLLAR_STATE_ERROR;
    }

    gps_l96_start_activity_tracking(gps_file_name);

    return DOG_COLLAR_STATE_GPS_TRACKING;
}

dog_collar_state_t handle_waiting_for_gps_fix_state(void) {

    /* Wait for GPS fix*/
    if(gps_l96_has_fix() ) {
        return DOG_COLLAR_STATE_GPS_FILE_CREATION;
    }

    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            
    }

    return DOG_COLLAR_STATE_LIGHT_SLEEP; 
}

dog_collar_state_t handle_gps_tracking_state(void) {

    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_PAUSED; 
    }

    gps_tracking_task(gps_file_name);

    return DOG_COLLAR_STATE_LIGHT_SLEEP;
}

dog_collar_state_t handle_gps_paused_state(void) {
    
    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_TRACKING; 
    }
    if (is_button_long_pressed()) {
        gps_l96_stop_activity_tracking();
        return DOG_COLLAR_STATE_NORMAL; 
    }

    return DOG_COLLAR_STATE_LIGHT_SLEEP;
}

dog_collar_state_t handle_wifi_sync_state(void) {
    static bool sync_started = false;
    static int64_t sync_start_time_us = 0;
    int64_t now = esp_timer_get_time();

    /* Start WIFI sync and set the start time */
    if (sync_started == false) {
        wifi_manager_reconnect(); 
        sync_start_time_us = now;
        sync_started = true;
        clear_button_press_states(); // For some reason we need to clear button press states here
    }

    // Allow button press to interrupt Wi-Fi sync and go to GPS acquiring state
    if (is_button_short_pressed() || is_button_long_pressed()) {
        sync_started = false;
        wifi_stop_all_services();
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }

    /* If time is up go deep sleep*/
    if ((now - sync_start_time_us) >= WIFI_SYNC_TIME_S * 1000 * 1000) {
        sync_started = false;
        wifi_stop_all_services();
        return DOG_COLLAR_STATE_DEEP_SLEEP;
    }

    // Stay in WIFI_SYNC until timeout or button press
    return DOG_COLLAR_STATE_WIFI_SYNC;
}

dog_collar_state_t  handle_light_sleep_state(void) {

    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
    esp_sleep_enable_timer_wakeup((uint64_t)LIGHT_SLEEP_TIME_S * 1000000);
    esp_light_sleep_start();

    return current_state; // Continues the work where it left off
}

dog_collar_state_t handle_deep_sleep_state(void) {
    
    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
    gps_l96_go_to_back_up_mode();

    // timer wakeup for periodic wake-ups
    esp_sleep_enable_timer_wakeup((uint64_t)DEEP_SLEEP_TIME_S * 1000000);

    button_interrupt_enable_wakeup();

    esp_deep_sleep_start();

    return DOG_COLLAR_STATE_NORMAL; // Never reached
}

dog_collar_state_t handle_error_state(void) {

    return DOG_COLLAR_STATE_INITIALIZING;
}

bool was_woken_by_button_press(void) {
    static bool checked_wakeup = false;

    /* First check if wakeup reason has been checked already*/
    if(checked_wakeup) {
        return false; // Already checked, no longer a wakeup event
    }

    /* Check the wakeup reason */
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
        checked_wakeup = true;
        return true;
    }

    checked_wakeup = true; 
    return false;   
}

static char *get_current_state_string(dog_collar_state_t state) {
    switch (state) {
        case DOG_COLLAR_STATE_INITIALIZING:
            return "INITIALIZING";
        case DOG_COLLAR_STATE_NORMAL:
            return "NORMAL";
        case DOG_COLLAR_STATE_LOW_BATTERY:
            return "LOW_BATTERY";
        case DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY:
            return "CRITICAL_LOW_BATTERY";
        case DOG_COLLAR_STATE_CHARGING:
            return "CHARGING";
        case DOG_COLLAR_STATE_GPS_ACQUIRING:
            return "GPS_ACQUIRING";
        case DOG_COLLAR_STATE_GPS_READY:
            return "GPS_READY";
        case DOG_COLLAR_STATE_GPS_FILE_CREATION:
            return "GPS_FILE_CREATION";
        case DOG_COLLAR_STATE_WAITING_FOR_GPS_FIX:
            return "WAITING_FOR_GPS_FIX";
        case DOG_COLLAR_STATE_GPS_TRACKING:
            return "GPS_TRACKING";
        case DOG_COLLAR_STATE_GPS_PAUSED:
            return "GPS_PAUSED";
        case DOG_COLLAR_STATE_WIFI_SYNC:
            return "WIFI_SYNC";
        case DOG_COLLAR_STATE_LIGHT_SLEEP:
            return "LIGHT_SLEEP";
        case DOG_COLLAR_STATE_DEEP_SLEEP:
            return "DEEP_SLEEP";
        case DOG_COLLAR_STATE_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

esp_err_t gps_tracking_task(char* gps_file_name) { 

    static char NMEA_sentence[NMEA_SENTENCE_BUF_SIZE] = {0};
    static uint8_t rx_buffer[UART_RX_BUF_SIZE] = {0};

    size_t read_len = 0;

    esp_err_t ret = uart_receive_cmd(rx_buffer, sizeof(rx_buffer), &read_len);

    if( ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "UART read timed out, no data received");
        return ESP_OK; // No data to process, just return
    }

    if (ret != ESP_OK || read_len == 0) {

        ESP_LOGE(TAG, "UART read failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = gps_l96_extract_and_process_nmea_sentences(rx_buffer, read_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NMEA extraction failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (gps_l96_has_fix() == false) {
        ESP_LOGW(TAG, "No valid GPS fix, not logging data.");
        return ESP_FAIL;
    }

    if( gps_file_name == NULL || strlen(gps_file_name) == 0) { // Add NULL for gps_file_name to just check if we have GPS fix
        ESP_LOGE(TAG, "GPS file name is not set");
        return ESP_ERR_INVALID_ARG;
    }

    ret = gps_l96_format_csv_line_from_data(NMEA_sentence, sizeof(NMEA_sentence));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CSV formatting failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = lfs_append_to_file(NMEA_sentence, gps_file_name);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "File append failed: %s", esp_err_to_name(ret));
    }
    return ret;
} 
