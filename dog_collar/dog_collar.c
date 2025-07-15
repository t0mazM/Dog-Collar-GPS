#include "dog_collar.h"

const char* TAGG = "DOG_COLLAR";
gps_state_t gps_state = GPS_STATE_IDLE;  // Default state on power on

esp_err_t dog_collar_components_init(void){

    collar_init_state_t init_state = {0};
    esp_err_t ret;

    // Initialize each component and update the init_state accordingly
    ret = ext_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize External Flash");
    }
    init_state.ext_flash_ready = (ret == ESP_OK);
    
    ret = gps_l96_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize GPS L96");
    }
    init_state.gps_l96_ready = (ret == ESP_OK);

    ret = battery_monitor_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize Battery Monitor");
    }
    init_state.batt_mon_ready = (ret == ESP_OK);

    ret = lfs_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize File System");
    }
    init_state.filesystem_ready = (ret == ESP_OK);

    ret = wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize Wi-Fi");
    }
    init_state.wifi_server_ready = (ret == ESP_OK);

    return ESP_OK;
}

static bool dog_collar_are_all_components_functional(const collar_init_state_t *dog_collar_init_state) {
    
    return dog_collar_init_state->ext_flash_ready &&
           dog_collar_init_state->gps_l96_ready &&
           dog_collar_init_state->batt_mon_ready &&
           dog_collar_init_state->filesystem_ready &&
           dog_collar_init_state->wifi_server_ready;
}