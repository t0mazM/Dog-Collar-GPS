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

    uint8_t jedec_id[3] = {0};

    // Call the function
    esp_err_t ret = spi_flash_read_jedec(jedec_id);


}