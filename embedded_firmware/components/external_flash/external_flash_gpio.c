/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "ext_flash.h"

static const char *TAG = "EXT_FLASH_GPIO";

esp_err_t config_gpio_pin(gpio_num_t GPIO_pin_handle, gpio_mode_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_pin_handle),
        .mode = mode,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&io_conf);
}

esp_err_t SPI_set_HOLD_WP_HIGH(void) {

    // Configure WP# as output high
    ESP_RETURN_ON_ERROR(config_gpio_pin(SPI_PIN_NUM_WP, GPIO_MODE_OUTPUT), TAG, "Failed to configure WP pin");
    ESP_RETURN_ON_ERROR(gpio_set_level(SPI_PIN_NUM_WP, 1), TAG, "Failed to set WP pin high");

    // Configure HOLD# as output high
    ESP_RETURN_ON_ERROR(config_gpio_pin(SPI_PIN_NUM_HOLD, GPIO_MODE_OUTPUT), TAG, "Failed to configure HOLD pin");
    ESP_RETURN_ON_ERROR(gpio_set_level(SPI_PIN_NUM_HOLD, 1), TAG, "Failed to set HOLD pin high");

    return ESP_OK;
}