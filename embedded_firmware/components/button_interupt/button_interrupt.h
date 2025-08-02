#ifndef BUTTON_INTERRUPT_H
#define BUTTON_INTERRUPT_H


#ifndef IRAM_ATTR
#define IRAM_ATTR /* Defines the IRAM attribute for interrupt handler */
#endif

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "../components/gpio_expander/gpio_expander.h"


#define BUTTON_GPIO         2    
#define DEBOUNCE_TIME_MS    50
#define LONG_PRESS_TIME_MS  1000

/**
 * @brief Initializes the button interrupt service.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t button_interrupt_init(void);

/**
 * @brief Checks if the button is currently pressed.
 * @return true if the button is pressed, false otherwise.
 */

 /**
  * @brief Returns bool value if button was short pressed
  * The bool value is saved in static global variable and once read is set back to false
  */
bool is_button_short_pressed(void);

 /**
  * @brief Returns bool value if button was long pressed
  * The bool value is saved in static global variable and once read is set back to false
  */
bool is_button_long_pressed(void);

/**
 * @brief Clears the button press states.
 * This function resets the button_short_pressed and button_long_pressed flags to false.
 *
 * @note Calling this function is used to not get bugs when the button is pressed multiple times in a short period.
 *       It is recommended to call this function after processing the button press.
 */
void clear_button_press_states(void);

#endif // BUTTON_INTERRUPT_H