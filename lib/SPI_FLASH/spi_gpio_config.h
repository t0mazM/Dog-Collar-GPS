#ifndef SPI_GPIO_CONFIG_H
#define SPI_GPIO_CONFIG_H

#include "driver/gpio.h"


#define SPI_PIN_NUM_WP   GPIO_NUM_1
#define SPI_PIN_NUM_HOLD GPIO_NUM_10

void hold_wp_setup(void);

#endif  // SPI_GPIO_CONFIG_H