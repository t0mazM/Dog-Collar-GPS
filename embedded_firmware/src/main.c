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
#include "../dog_collar/dog_collar_state_machine/led_management/LED_management.h"


void app_main() {

    /* Create FreeRTOS tasks */
    xTaskCreate(state_machine_task, "state_machine_task", 4096, NULL, 1, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 2, NULL);

    /* Delete the main task */
    vTaskDelete(NULL);
}