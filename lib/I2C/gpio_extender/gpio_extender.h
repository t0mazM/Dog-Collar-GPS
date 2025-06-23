#ifndef GPIO_EXTENDER_H
#define GPIO_EXTENDER_H

#include <i2c.h>

typedef enum {
    LED_RED    = 0b00100000, // bit 5
    LED_YELLOW = 0b01000000, // bit 6
    LED_GREEN  = 0b10000000, // bit 7
} led_colour_t;

#define PCF8574_ADDR 0x27
void gpio_init(void);
void gpio_turn_on_leds(uint8_t led_mask);
void gpio_turn_off_leds(uint8_t led_mask);
#endif