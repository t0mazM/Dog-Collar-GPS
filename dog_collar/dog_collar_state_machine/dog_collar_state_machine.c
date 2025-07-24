#include "dog_collar_state_machine.h"

static char *get_current_state_string(dog_collar_state_t state);
static void enter_light_sleep(uint64_t sleep_time_us);
static esp_err_t gps_tracking_task(char *gps_file_name);

static const char *TAG = "DOG_COLLAR_STATE_MACHINE";

/* Global variables for dog collar state machine */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;
static char gps_file_name[LFS_MAX_FILE_NAME_SIZE] = {0};

dog_collar_state_t dog_collar_state_machine_run(void) {


    printf("Current state: %s\n", get_current_state_string(current_state));

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
        case DOG_COLLAR_STATE_ERROR:
            current_state = handle_error_state();
            break;
        default:
            // Handle unexpected state
            current_state = DOG_COLLAR_STATE_ERROR;
    }
    current_state = battery_management_routine(current_state);
    return current_state;
}

dog_collar_state_t battery_management_routine(dog_collar_state_t current_state) {
    esp_err_t ret = battery_monitor_update_battery_data();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update battery data");
        return DOG_COLLAR_STATE_ERROR;
    }

    if (battery_status_flags.charging && current_state != DOG_COLLAR_STATE_CHARGING) {
        ESP_LOGI(TAG, "Battery is charging");
        // return DOG_COLLAR_STATE_CHARGING; commened out for easy testing
    }

    if (battery_data.soc >= 0.0f && battery_data.soc <= 10.0f && current_state != DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY) {
        ESP_LOGW(TAG, "Battery is critically low");
        return DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY;
    } else if (battery_data.soc > 10.0f && battery_data.soc <= 20.0f && current_state != DOG_COLLAR_STATE_LOW_BATTERY) {
        ESP_LOGW(TAG, "Battery is low");
        return DOG_COLLAR_STATE_LOW_BATTERY;
    }

    // If battery is OK, stay in the current state (do not go to NORMAL)
    return current_state;
}

dog_collar_state_t handle_initializing_state(void) {
   

    esp_err_t init_result = dog_collar_components_init();

    if (init_result == ESP_OK) {
        return DOG_COLLAR_STATE_NORMAL;
    } else {
        /* We can check the specific component that failed in global struct collar_init_state */
        return DOG_COLLAR_STATE_ERROR; 
    }
}

dog_collar_state_t handle_normal_state(void) {
    static bool normal_started = false;
    static int64_t normal_start_time_us = 0;
    int64_t now_us = esp_timer_get_time();

    // 1) Check if button was pressed
    if (is_button_short_pressed()) {
        vTaskDelay(pdMS_TO_TICKS(WAIT_AFTER_USER_PRESS_MS)); // Wait a bit and clear the button state
        clear_button_press_states();
        normal_started = false; // Set flag to false so that we can start the timer again on next entry
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }

    // 2) Start enter wifi_sync state every WIFI_SYNC_PERIODIC_TIME_S seconds
    // 2.a) On first normal state entry, set the start time
    if (!normal_started) {
        normal_start_time_us = now_us;
        normal_started = true;
    }

    // 2.b) If time is up, go to WIFI_SYNC state
    if ((now_us - normal_start_time_us) >= WIFI_SYNC_PERIODIC_TIME_S * 1000 * 1000) {
        normal_started = false; // Set flag to false so that we can start the timer again on next entry
        return DOG_COLLAR_STATE_WIFI_SYNC;
    }

    /** 3) If nothing happened, go to light sleep and stay in NORMAL state 
        Set normal_started to false so that we can start the timer again on next entry */
    enter_light_sleep(SLEEP_TIME_S * 1000 * 1000);

    gpio_toggle_leds(LED_GREEN); //used to debug. TODO: add a proper LED blinking routine

    return DOG_COLLAR_STATE_NORMAL;
}

dog_collar_state_t handle_low_battery_state(void) {

    return DOG_COLLAR_STATE_LOW_BATTERY;
}

dog_collar_state_t handle_critical_low_battery_state(void) {

    return DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY;
}

dog_collar_state_t handle_charging_state(void) {

    return DOG_COLLAR_STATE_CHARGING;
}

dog_collar_state_t handle_gps_acquiring_state(void) {
    ESP_LOGI(TAG, "Waiting for GPS fix or user input");
    vTaskDelay(pdMS_TO_TICKS(1000)); //Used to test if 1Hz is working or not
    
    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_FILE_CREATION; // go and create a GPS file
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            // go back to normal state
    }

    if(gps_l96_has_fix() ) {
        return DOG_COLLAR_STATE_GPS_READY;
    }

    return DOG_COLLAR_STATE_GPS_ACQUIRING; // Loop back to GPS_ACQUIRING
}

dog_collar_state_t handle_gps_ready_state(void) {
    ESP_LOGI(TAG, "GPS is ready. Waiting for user input");
    gps_l96_start_recording();

    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_FILE_CREATION; // go and create a GPS file
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            // go back to normal state
    }

    //TODO: Add LED animation for GPS ready state
    return DOG_COLLAR_STATE_GPS_READY;
}

dog_collar_state_t handle_gps_file_creation_state(void) {

    
    esp_err_t ret = lfs_create_new_csv_file(gps_file_name, sizeof(gps_file_name));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create GPS file");
        return DOG_COLLAR_STATE_ERROR;
    }

    return DOG_COLLAR_STATE_GPS_TRACKING;
}

dog_collar_state_t handle_gps_tracking_state(void) {

    ESP_LOGI(TAG, "Tracking GPS");

    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_PAUSED; 
    }


    gps_tracking_task(gps_file_name);
    enter_light_sleep( 500 * 1000); //500ms //TODO test sleep. Test if uart wakes up the sleep

    return DOG_COLLAR_STATE_GPS_TRACKING;
}

dog_collar_state_t handle_gps_paused_state(void) {
    
    ESP_LOGI(TAG, "GPS is paused. Waiting for user input");

    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_TRACKING; // go and create a GPS file
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            // go back to normal state
    }

    //TODO: Add LED animation for GPS paused state
    return DOG_COLLAR_STATE_GPS_PAUSED;
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
    }

    // Allow button press to interrupt Wi-Fi sync and go to GPS acquiring state
    if (is_button_short_pressed() || is_button_long_pressed()) {
        sync_started = false;
        wifi_stop_all_services();
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }

    /* If time is up go back to normal state*/
    if ((now - sync_start_time_us) >= WIFI_SYNC_TIME_S * 1000 * 1000) {
        sync_started = false;
        wifi_stop_all_services();
        return DOG_COLLAR_STATE_NORMAL;
    }

    // Stay in WIFI_SYNC until timeout or button press
    return DOG_COLLAR_STATE_WIFI_SYNC;
}


dog_collar_state_t handle_error_state(void) {
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_toggle_leds(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    return DOG_COLLAR_STATE_INITIALIZING;
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
        case DOG_COLLAR_STATE_GPS_TRACKING:
            return "GPS_TRACKING";
        case DOG_COLLAR_STATE_GPS_PAUSED:
            return "GPS_PAUSED";
        case DOG_COLLAR_STATE_WIFI_SYNC:
            return "WIFI_SYNC";
        case DOG_COLLAR_STATE_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

static void enter_light_sleep(uint64_t sleep_time_us) {

    // esp_sleep_enable_timer_wakeup(sleep_time_us);
    // esp_sleep_enable_gpio_wakeup();
    // gpio_wakeup_enable(BUTTON_GPIO, GPIO_INTR_LOW_LEVEL); // Wake on button press (active low)
    // esp_sleep_enable_uart_wakeup(UART_PORT_NUM); // Wake on UART activity TODO:test this with sleep while gps is tracking

    // esp_light_sleep_start();

    /* The light sleep functionality is currently disabled as after 
    waking up the usb uart is not reinitialized properly and is not working*/

    vTaskDelay(pdMS_TO_TICKS(sleep_time_us)); // Simulate light sleep with delay

    
}

esp_err_t gps_tracking_task(char *gps_file_name) { 
    static uint8_t rx_buffer[UART_RX_BUF_SIZE];
    memset(rx_buffer, 0, sizeof(rx_buffer));
    size_t read_len = 0;
    static char NMEA_sentence[NMEA_SENTENCE_BUF_SIZE] = {0};

    esp_err_t ret = uart_receive_cmd(rx_buffer, sizeof(rx_buffer), &read_len);
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
