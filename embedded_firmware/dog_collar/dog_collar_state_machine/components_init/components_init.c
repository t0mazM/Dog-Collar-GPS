#include "components_init.h"

const char* TAGG = "DOG_COLLAR";
collar_init_state_t collar_init_state = {0};

static void dog_collar_log_init_state(void);

esp_err_t dog_collar_components_init(void){

    esp_err_t ret;
    esp_err_t overall_init_result = ESP_OK;

    ret = button_interrupt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize Button Interrupt");
        overall_init_result = ret;
    }

    // Initialize each component and update the init_state accordingly
    ret = ext_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize External Flash");
        overall_init_result = ret;
    }
    collar_init_state.ext_flash_ready = (ret == ESP_OK);
    
    ret = gps_l96_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize GPS L96");
        overall_init_result = ret;
    }
    collar_init_state.gps_l96_ready = (ret == ESP_OK);

    ret = battery_monitor_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize Battery Monitor");
        overall_init_result = ret;
    }
    collar_init_state.batt_mon_ready = (ret == ESP_OK);

    ret = lfs_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAGG, "Failed to initialize File System");
        overall_init_result = ret;
    }
    collar_init_state.filesystem_ready = (ret == ESP_OK);

    dog_collar_log_init_state();
    return overall_init_result;
}

static bool dog_collar_are_all_components_functional() {

    return collar_init_state.ext_flash_ready &&
           collar_init_state.gps_l96_ready &&
           collar_init_state.batt_mon_ready &&
           collar_init_state.filesystem_ready;
}

int dog_collar_get_status_string(char *string_buffer, size_t string_buffer_size) {

    int written = snprintf(string_buffer, string_buffer_size,
        "\n============= Dog Collar System Status ==============\n"
        "External Flash:   %s\n"
        "GPS Module:       %s\n" 
        "Battery Monitor:  %s\n"
        "File System:      %s\n"
        "Overall Status:   %s\n"
        "=========================================================\n",
        collar_init_state.ext_flash_ready ? "OK" : "FAILED",
        collar_init_state.gps_l96_ready ? "OK" : "FAILED",
        collar_init_state.batt_mon_ready ? "OK" : "FAILED",
        collar_init_state.filesystem_ready ? "OK" : "FAILED",
        dog_collar_are_all_components_functional(collar_init_state) ? "All OK" : "Yeah, there are issues"
    );

    // Return -1 if buffer was too small
    return (written >= 0 && written < (int)string_buffer_size) ? written : -1;
}

static void dog_collar_log_init_state(void) {
    char status_string[256];
    int result = dog_collar_get_status_string(status_string, sizeof(status_string));
    if (result > 0) {
        ESP_LOGI(TAGG, "%s", status_string);
    } else {
        ESP_LOGE(TAGG, "Failed to get status string");
    }
}