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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "../dog_collar/components_init.h"


void app_main() {

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize


    dog_collar_components_init(); // Initialize all components


    ext_flash_read_jedec_data();

    uint8_t status_reg = 0;
    ext_flash_read_status_register(&status_reg);
    ext_flash_write_enable();
    ext_flash_read_status_register(&status_reg);
    ext_flash_wait_for_idle(2000);
    //file_system_test();



    //vTaskDelay(15000 / portTICK_PERIOD_MS); 
    //wifi_stop_all_services_retry(3)


    while(1) {
        battery_monitor_update_battery_data();
        gpio_toggle_leds(LED_RED | LED_GREEN);
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