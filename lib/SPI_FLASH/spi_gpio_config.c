#include "spi_gpio_config.h"

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

void hold_wp_setup(void) {

    // Configure WP# as output high
    config_gpio_pin(SPI_PIN_NUM_WP, GPIO_MODE_OUTPUT);
    gpio_set_level(SPI_PIN_NUM_WP, 1);

    // Configure HOLD# as output high
    config_gpio_pin(SPI_PIN_NUM_HOLD, GPIO_MODE_OUTPUT);
    gpio_set_level(SPI_PIN_NUM_HOLD, 1);
}