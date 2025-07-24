#include <stdio.h>
#include <i2c.h>
#include <uart.h>
#include "external_flash/ext_flash.h"
#include "battery_monitor/battery_monitor.h"
#include "gpio_expander/gpio_expander.h"
#include "gps_l96/gps_l96.h"
#include "gps_l96/nmea_commands.h"
#include "file_system_littlefs/file_system_littlefs.h"
#include "network_services/wifi_manager.h"
#include "../dog_collar/dog_collar_state_machine/dog_collar_state_machine.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "../dog_collar/dog_collar_state_machine/components_init/components_init.h"


void app_main() {

    vTaskDelay(pdMS_TO_TICKS(5000)); 

    while(true) {
        dog_collar_state_machine_run();
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }



}