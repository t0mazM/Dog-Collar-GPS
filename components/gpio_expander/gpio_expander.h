#ifndef GPIO_EXPANDER_H
#define GPIO_EXPANDER_H

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
} led_colour_t;


esp_err_t gpio_init(void);
/**
 * @brief Synchronizes the GPIO state off the PCF8574 GPIO expander.
 * 
 * This function reds the states off all GPIO pins from the PCF8574 GPIO expander
 * and updates the global variable `gpio_output_state` with the current state.
 * 
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t gpio_sync_state(void);

/**
 * @brief Toggles the leds specified by the led_mask.
 * 
 * This function updates the global variable `gpio_output_state` 
 * and toggles the leds specified by the `led_mask`.
 * 
 * @param led_mask Bitmask specifying which LEDs to turn on
 */
esp_err_t gpio_toggle_leds(uint8_t led_mask);

void gpio_turn_on_leds(uint8_t led_mask);
void gpio_turn_off_leds(uint8_t led_mask);
esp_err_t gpio_read_inputs(uint8_t *input_state);
void gpio_reset_gps(void);
esp_err_t gps_force_on_set(bool enable);
#endif // GPIO_EXPANDER_H