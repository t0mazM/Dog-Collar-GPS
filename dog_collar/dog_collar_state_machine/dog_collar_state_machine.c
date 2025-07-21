#include "dog_collar_state_machine.h"

/* Default state at startup is to initialize the system */
static dog_collar_state_t current_state = DOG_COLLAR_STATE_INITIALIZING;

dog_collar_state_t dog_collar_state_machine_run(void) {
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
        case DOG_COLLAR_STATE_CHARGING:
            current_state = handle_charging_state();
            break;
        case DOG_COLLAR_STATE_GPS_ACQUIRING:
            current_state = handle_gps_acquiring_state();
            break;
        case DOG_COLLAR_STATE_GPS_READY:
            current_state = handle_gps_ready_state();
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