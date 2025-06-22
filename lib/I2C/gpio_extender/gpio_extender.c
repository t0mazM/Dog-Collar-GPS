#include "gpio_extender.h"

void gpio_turn_on_led(void) {
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, 0b00011111); // Assuming 0x01 turns on the LED


}