#ifndef BUTTON_INTERRUPT_H
#define BUTTON_INTERRUPT_H


#ifndef IRAM_ATTR
#define IRAM_ATTR /* Defines the IRAM attribute for interrupt handler */
#endif

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "../components/gpio_expander/gpio_expander.h"


#define BUTTON_GPIO         9    
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
bool is_button_short_pressed(void);
bool is_button_long_pressed(void);

#endif // BUTTON_INTERRUPT_H