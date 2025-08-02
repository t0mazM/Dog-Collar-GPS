/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef EXT_FLASH_GPIO_H
#define EXT_FLASH_GPIO_H

#include "driver/gpio.h"

#define SPI_PIN_NUM_WP   GPIO_NUM_1
#define SPI_PIN_NUM_HOLD GPIO_NUM_10

/**
 * @brief Initializes and sets HIGH the SPI flash chip's WP and HOLD pins.
 *
 * This function sets both pins to HIGH to test the SPI flash chip's
 * functionality.
 *
 * The pins could have been physically connected to the 3.3V, but were forgotten during design phase so this function
 * ensures they are set to HIGH.
 *
 * @note This function should be called after initializing the GPIO expander.
 */
esp_err_t SPI_set_HOLD_WP_HIGH(void);

#endif  // SPI_GPIO_CONFIG_H