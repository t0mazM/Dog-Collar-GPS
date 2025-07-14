#include <stdio.h>
#include <i2c.h>
#include <uart.h>
#include <ext_flash.h>
#include "battery_monitor.h"
#include "gpio_expander.h"
#include "gps_l96.h"
#include "nmea_commands.h"
#include "file_system_littlefs.h"
#include "wifi_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"


void app_main() {

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize

    i2c_init();
    ext_flash_init();
    gpio_init();
    uart_init();
    gps_l96_init();
    lfs_mount_filesystem(true);
    nvs_flash_init();
    wifi_connect_and_start_services();



    uint8_t jedec_id[3] = {0};
    ext_flash_read_jedec_data(jedec_id);

    uint8_t status_reg = 0;
    ext_flash_read_status_register(&status_reg);
    ext_flash_write_enable();
    ext_flash_read_status_register(&status_reg);
    ext_flash_wait_for_idle(2000);
    //file_system_test();

    vTaskDelay(15000 / portTICK_PERIOD_MS); 
    wifi_stop_all_services_retry(10);


    while(1) {
        battery_monitor_update_battery_data(&battery_data);
        gpio_turn_on_leds(LED_RED | LED_YELLOW | LED_GREEN);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
        gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }


    //lfs_delete_file("/dog_run__353.csv");
    lfs_list_directory("/", NULL, 0); 
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