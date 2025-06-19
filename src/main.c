#include <stdio.h>
#include <i2c.h>
#include <uart.h>
#include <spi_flash.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    vTaskDelay(pdMS_TO_TICKS(5000)); //It takes some time for the serial monitor to start up
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI("MAIN", "Booting application...");

    init_i2c();
    init_spi_flash();

    while(1){
    uint8_t jedec_id[3] = {0};

    // Call the function
    esp_err_t ret = spi_flash_read_jedec(jedec_id);

    if (ret == ESP_OK) {
    
    printf("Manufacturer ID: 0x%02X\n", jedec_id[0]);
    printf("Memory Type:     0x%02X\n", jedec_id[1]);
    printf("Capacity:        0x%02X\n", jedec_id[2]);
    } else {
    printf("Failed to read JEDEC ID: %d\n", ret);
    }

        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay for 1 second
    }
}