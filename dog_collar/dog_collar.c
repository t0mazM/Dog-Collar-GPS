#include "dog_collar.h"

const char* TAGG = "dog_collar";
gps_state_t gps_state = GPS_STATE_IDLE;  // Default state on power on

esp_err_t dog_collar_components_init(void){

    collar_init_state_t init_state = {0};

    // Initialize each component and update the init_state accordingly

    ESP_RETURN_ON_ERROR(ext_flash_init(), TAGG, "Failed to initialize External Flash");
    init_state.flash_ready = true;

    ESP_RETURN_ON_ERROR(gps_l96_init(), TAGG, "Failed to initialize GPS L96");
    init_state.gps_ready = true;

    ESP_RETURN_ON_ERROR(battery_monitor_init(), TAGG, "Failed to initialize Battery Monitor");
    init_state.battery_ready = true;

    ESP_RETURN_ON_ERROR(lfs_init(), TAGG, "Failed to initialize File System");
    init_state.filesystem_ready = true;

    ESP_RETURN_ON_ERROR(wifi_init(), TAGG, "Failed to initialize Wi-Fi");
    init_state.wifi_server_ready = true;

    return ESP_OK;
}