#include <stdio.h>
#include <i2c.h>


#include <uart.h>
#include <ext_flash.h>
#include "battery_monitor/battery_monitor.h"
#include "gpio_extender/gpio_extender.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize

    i2c_init();
    ext_flash_init();
    gpio_init();

    battery_monitor_update_battery_data(&battery_data);
    gpio_read_inputs();

    uint8_t jedec_id[3] = {0};
    // Call the function
    ext_flash_read_jedec_data(jedec_id);


}