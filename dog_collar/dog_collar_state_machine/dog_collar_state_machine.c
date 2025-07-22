#include "dog_collar_state_machine.h"

static const char *TAG = "DOG_COLLAR_STATE_MACHINE";

/* Default state at startup is to initialize the system */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;

dog_collar_state_t dog_collar_state_machine_run(void) {

    current_state = battery_management_routine(current_state);

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

    return DOG_COLLAR_STATE_WIFI_SYNC;
}


dog_collar_state_t handle_error_state(void) {
    return DOG_COLLAR_STATE_ERROR;
}