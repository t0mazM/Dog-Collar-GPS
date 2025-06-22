#include <stdio.h>
#include <i2c.h>


#include <uart.h>
#include <ext_flash.h>
#include "battery_monitor/battery_monitor.h"
#include "gpio_extender/gpio_extender.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {

    i2c_init();
    ext_flash_init();

    battery_monitor_update_battery_data(&battery_data);
    gpio_turn_on_led();

    uint8_t jedec_id[3] = {0};
    // Call the function
    ext_flash_read_jedec_data(jedec_id);


}