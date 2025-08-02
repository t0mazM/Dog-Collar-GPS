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

/** Get the current output state of the GPIO expander.
 * @return The current output state.
 */
uint8_t gpio_expander_get_output_state(void);

/**
 * @brief Sets the output state of the GPIO expander.
 * 
 * This function updates the global variable `gpio_output_state` with the new state.
 * It also writes the new state to the PCF8574 GPIO expander.
 * 
 * @param state The new output state to set.
 */
void gpio_expander_update_output_state(uint8_t state);

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

/**
 * @brief Turns on the LEDs specified by the led_mask.
 * 
 * This function updates the global variable `gpio_output_state` 
 * and turns on the LEDs specified by the `led_mask`.
 * 
 * @note You can turn on multiple LEDs at once by: gpio_turn_on_leds(LED_GREEN | LED_YELLOW | LED_RED);
 * 
 * @param led_mask Bitmask specifying which LEDs to turn on
 */
void gpio_turn_on_leds(uint8_t led_mask);

/**
 * @brief Turns off the LEDs specified by the led_mask.
 * 
 * This function updates the global variable `gpio_output_state` 
 * and turns off the LEDs specified by the `led_mask`.
 * 
 * @note You can turn off multiple LEDs at once by: gpio_turn_off_leds(LED_GREEN | LED_YELLOW | LED_RED);
 * 
 * @param led_mask Bitmask specifying which LEDs to turn off
 */
void gpio_turn_off_leds(uint8_t led_mask);

/**
 * @brief Reads the state of the inputs from the GPIO expander.
 * 
 * This function reads the state of the inputs from the PCF8574 GPIO expander
 * and updates the global variable `gpio_input_state` with the current state.
 * 
 * @param input_state Pointer to a variable where the input state will be stored.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t gpio_read_inputs(uint8_t *input_state);

/**
 * @brief Resets the GPS module by toggling the GPS_RESET pin.
 * 
 * This function toggles the GPS_RESET pin to reset the GPS module.
 * It first sets the pin low, waits 100 ms and then sets it high again. It also waits 1 second to allow the GPS module to reset.
 * 
 * @note The waits are necessary to ensure the GPS module has enough time to reset.
 */

#endif // GPIO_EXPANDER_H