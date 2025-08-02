#ifndef LED_MANAGEMENT_H
#define LED_MANAGEMENT_H


#include "gpio_expander/gpio_expander.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

#include "../dog_collar_state_machine.h"

/**
 * @brief Sets the LED pattern based on the current dog collar state.
 * 
 * It sets the static global variable led_current_state and starts the LED blinking pattern.
 * 
 * @param state The current state of the dog collar.
 */
void led_management_set_pattern(dog_collar_state_t state);

/**
 * @brief Task that manages the LED blinking patterns based on the current state.
 * 
 * This task runs indefinitely and updates the LEDs.
 * Run this task in a FreeRTOS task to manage the LED patterns.
 * 
 * @note To change the LED pattern, call led_management_set_pattern() with the desired state.
 */
void led_task(void *pvParameters);


#endif // LED_MANAGEMENT_H