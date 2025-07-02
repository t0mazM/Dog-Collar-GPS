#include <stdio.h>
#include <i2c.h>
#include <uart.h>
#include <ext_flash.h>
#include "battery_monitor/battery_monitor.h"
#include "gpio_extender/gpio_extender.h"
#include "gps_l96/gps_l96.h"
#include "gps_l96/nmea_commands.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void app_main() {

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize

    i2c_init();
    ext_flash_init();
    gpio_init();
    uart_init();
    gps_l96_init();

    battery_monitor_update_battery_data(&battery_data);
    gpio_turn_on_leds(LED_RED | LED_YELLOW | LED_GREEN);
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);

    uint8_t jedec_id[3] = {0};
    ext_flash_read_jedec_data(jedec_id);

    uint8_t status_reg = 0;
    ext_flash_read_status_register(&status_reg);
    ext_flash_write_enable();
    ext_flash_read_status_register(&status_reg);


    
    //gps_l96_go_to_standby_mode();
    // gps_l96_go_to_back_up_mode();
    // vTaskDelay(5000 / portTICK_PERIOD_MS); 
    // gps_l96_read_task();
    // gps_l96_start_recording();

    // while(1){
    //     gps_l96_read_task();
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); 
    // }
}