#ifndef SPI_GPIO_CONFIG_H
#define SPI_GPIO_CONFIG_H

#include "driver/gpio.h"


#define SPI_PIN_NUM_WP   GPIO_NUM_1
#define SPI_PIN_NUM_HOLD GPIO_NUM_10

/**
 * @brief Initializes and sets HIGH the SPI flash chip's WP and HOLD pins.
 *
 * This function sets both pins to HIGH to test the SPI flash chip's
 * functionality.
 *
 * @note Used only for testing purposes.
 */
void SPI_set_HOLD_WP_HIGH(void);

#endif  // SPI_GPIO_CONFIG_H