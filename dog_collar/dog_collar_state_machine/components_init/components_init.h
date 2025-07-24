#ifndef COMPONENTS_INIT_H
#define COMPONENTS_INIT_H

/* Includes from standard libraries */
#include <stdbool.h>

/* Includes from ESP-IDF */
#include "esp_err.h"

/* Includes from dog collar components */
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

esp_err_t dog_collar_components_init(void);
int dog_collar_get_status_string(char *string_buffer, size_t string_buffer_size);






#endif // COMPONENTS_INIT_H