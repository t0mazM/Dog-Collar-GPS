#include "gpio_extender.h"

static uint8_t gpio_output_state = 0xFF; // All LEDs off initially (bits all high)



// Initialize PCF8574 pins (all LEDs off)
void gpio_init(void) {
    gpio_output_state = 0xFF;
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

// Turn on one or multiple LEDs by clearing bits
void gpio_turn_on_leds(uint8_t led_mask) {
    gpio_output_state &= ~led_mask; // clear bits to turn ON LEDs
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

// Turn off one or multiple LEDs by setting bits
void gpio_turn_off_leds(uint8_t led_mask) {
    gpio_output_state |= led_mask; // set bits to turn OFF LEDs
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}
