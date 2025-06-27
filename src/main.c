#include <stdio.h>
#include <i2c.h>
#include <uart.h>
#include <ext_flash.h>
#include "battery_monitor/battery_monitor.h"
#include "gpio_extender/gpio_extender.h"
#include "gps_l96/gps_l96.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void app_main() {

    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize

    i2c_init();
    ext_flash_init();
    gpio_init();
    uart_init();

    battery_monitor_update_battery_data(&battery_data);
    gpio_read_inputs();
    gpio_turn_on_leds(LED_RED | LED_YELLOW | LED_GREEN);
    gpio_read_inputs();
    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
    gpio_read_inputs();

    uint8_t jedec_id[3] = {0};
    ext_flash_read_jedec_data(jedec_id);




    
    const char *cmd = "$PMTK605*31\r\n"; // set update rate to 1Hz
    ESP_ERROR_CHECK(uart_send_cmd(cmd, strlen(cmd)));

    uint8_t rx_buffer[255];
    size_t read_len = 0;


    while (1) {
    uart_receive_cmd(rx_buffer, sizeof(rx_buffer), &read_len);
    gps_l96_extract_and_process_nmea_sentences(rx_buffer, read_len);

    vTaskDelay(pdMS_TO_TICKS(10));
    }
}