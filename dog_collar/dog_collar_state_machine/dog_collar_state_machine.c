#include "dog_collar_state_machine.h"

static char *get_current_state_string(dog_collar_state_t state);
static void enter_light_sleep(uint64_t sleep_time_us);

static const char *TAG = "DOG_COLLAR_STATE_MACHINE";
static int64_t last_wifi_sync_time_us = 0; // Last wifi sync time in microseconds

/* Default state at startup is to initialize the system */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;

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
    int64_t now_us = esp_timer_get_time();

    // If first entry, set the timer
    if (last_wifi_sync_time_us == 0) {
        last_wifi_sync_time_us = now_us;
    }

    if ((now_us - last_wifi_sync_time_us) >= WIFI_SYNC_TIME_S * 1000 * 1000) { 
        last_wifi_sync_time_us = now_us;
        return DOG_COLLAR_STATE_WIFI_SYNC;
    }

    enter_light_sleep(SLEEP_TIME_S * 1000);
    // Remain in NORMAL state
    gpio_toggle_leds(LED_GREEN);
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

    return DOG_COLLAR_STATE_GPS_ACQUIRING;
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

    wifi_manager_reconnect();
    return DOG_COLLAR_STATE_NORMAL;
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

    // esp_sleep_enable_timer_wakeup(3000000);

    // ESP_LOGI(TAG, "Entering light sleep for %llu us", sleep_time_us);
    // esp_light_sleep_start();

    // ESP_LOGI(TAG, "Woke up from light sleep");


    /* The light sleep functionality is currently disabled as after 
    waking up the usb uart is not reinitialized properly and is not working*/

    vTaskDelay(pdMS_TO_TICKS(sleep_time_us / 1000)); 
}