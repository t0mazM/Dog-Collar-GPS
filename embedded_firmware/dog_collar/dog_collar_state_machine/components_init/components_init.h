/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef COMPONENTS_INIT_H
#define COMPONENTS_INIT_H


#include <stdbool.h>
#include "esp_err.h"
#include "../components/battery_monitor/battery_monitor.h"
#include "../components/external_flash/ext_flash.h"
#include "../components/file_system_littlefs/file_system_littlefs.h"
#include "../components/gpio_expander/gpio_expander.h"
#include "../components/gps_l96/gps_l96.h"
#include "../components/network_services/wifi_manager.h"
#include "../components/button_interupt/button_interrupt.h"


typedef struct {
    bool ext_flash_ready;
    bool gps_l96_ready;
    bool batt_mon_ready;
    bool filesystem_ready;
} collar_init_state_t;

/**
 * @brief Initializes all components of the dog collar.
 * 
 * This function initializes the following components:
 * - External Flash
 * - GPS L96 module
 * - Battery Monitor
 * - File System (LittleFS)
 * - GPIO Expander
 * - Wi-Fi Manager
 * - Button Interrupt
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t dog_collar_components_init(void);

/**
 * @brief Logs the current state.
 * 
 * This function logs the current state of each component to the ESP-IDF log system.
 */
int dog_collar_get_status_string(char *string_buffer, size_t string_buffer_size);






#endif // COMPONENTS_INIT_H