#ifndef GPIO_EXTENDER_H
#define GPIO_EXTENDER_H

#include <i2c.h>
#define PCF8574_ADDR 0x27


typedef enum {
    // LEDS
    LED_RED    = 0b10000000, // GP7 -> Output
    LED_YELLOW = 0b01000000, // GP6 -> Output
    LED_GREEN  = 0b00100000, // GP5 -> Output
    // GPS L96-M33 module
    GEO_FENCE    = 0b00000001, // GP0 -> Input
    GPS_RESET    = 0b00000010, // GP1 -> Output
    GPS_JAM_IND  = 0b00000100, // GP2 -> Input
    GPS_FORCE_ON = 0b00001000, // GP3 -> Output
} gpio_bit_t;


void gpio_init(void);
void gpio_turn_on_leds(uint8_t led_mask);
void gpio_turn_off_leds(uint8_t led_mask);
void gpio_read_inputs(void);
esp_err_t gpio_set_pin_force(bool on);
#endif