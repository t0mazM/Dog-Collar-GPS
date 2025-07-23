#include "dog_collar_state_machine.h"

static char *get_current_state_string(dog_collar_state_t state);
static void enter_light_sleep(uint64_t sleep_time_us);

static const char *TAG = "DOG_COLLAR_STATE_MACHINE";

/* Default state at startup is to initialize the system */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;

dog_collar_state_t dog_collar_state_machine_run(void) {


    printf("Current state: %s\n", get_current_state_string(current_state));
    printf("Short press: %d, Long press: %d\n", is_button_short_pressed(), is_button_long_pressed()); 

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
        return DOG_COLLAR_STATE_CHARGING;
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
        vTaskDelay(pdMS_TO_TICKS(500)); // Wait a bit and clear the button state
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
        normal_started = false;
        return DOG_COLLAR_STATE_WIFI_SYNC;
    }

    /** 3) If nothing happened, go to light sleep and stay in NORMAL state 
        Set normal_started to false so that we can start the timer again on next entry */
    enter_light_sleep(SLEEP_TIME_S * 1000 * 1000);
    normal_started = false;

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

    
    if (is_button_short_pressed()) { 
        return DOG_COLLAR_STATE_GPS_FILE_CREATION; // go and create a GPS file
    }
    if (is_button_long_pressed()) {
        return DOG_COLLAR_STATE_NORMAL;            // go back to normal state
    }

    return DOG_COLLAR_STATE_GPS_ACQUIRING; // Loop back to GPS_ACQUIRING
}

dog_collar_state_t handle_gps_ready_state(void) {

    return DOG_COLLAR_STATE_GPS_READY;
}

dog_collar_state_t handle_gps_file_creation_state(void) {

    return DOG_COLLAR_STATE_GPS_FILE_CREATION;
}

dog_collar_state_t handle_gps_tracking_state(void) {

    return DOG_COLLAR_STATE_GPS_TRACKING;
}

dog_collar_state_t handle_gps_paused_state(void) {

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
        return DOG_COLLAR_STATE_GPS_ACQUIRING;
    }

    /* If time is up go back to normal state*/
    if ((now - sync_start_time_us) >= WIFI_SYNC_TIME_S * 1000 * 1000) {
        sync_started = false;
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

    // esp_light_sleep_start();

    /* The light sleep functionality is currently disabled as after 
    waking up the usb uart is not reinitialized properly and is not working*/

    vTaskDelay(pdMS_TO_TICKS(5000)); // Simulate light sleep with delay

    
}