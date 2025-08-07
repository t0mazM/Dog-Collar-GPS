/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "button_interrupt.h"

static const char *TAG = "BUTTON_HANDLER";
static bool button_interrupt_initialized = false;

/* Button press state variables */
static volatile bool button_short_pressed = false;
static volatile bool button_long_pressed = false;
static int64_t button_press_time = 0;
static esp_timer_handle_t debounce_timer;

/* Button debounce timer callback - function is saved to IRAM(internal RAM) */
static void IRAM_ATTR button_isr_handler(void* arg) {
    esp_timer_start_once(debounce_timer, DEBOUNCE_TIME_MS * 1000);
}

/**
 * @brief Button press evaluation callback function
 * This function is called when the debounce timer expires.
 * It reads the current button state and determines if it was a short or long press.
 */
static void debounce_timer_callback(void* arg) {
    /* Read current button state */
    int level = gpio_get_level(BUTTON_GPIO);
    int64_t now = esp_timer_get_time() / 1000; // ms
    int64_t press_duration = 0;

    /* If button is still pressed, record press time */
    if (level == 0) {
        button_press_time = now;
        return;
    }

    /* Button released - determine press duration */
    press_duration = now - button_press_time;
    
    if (press_duration >= LONG_PRESS_TIME_MS) {
        button_long_pressed = true;
        ESP_LOGI(TAG, "Button long press detected");
        gpio_toggle_leds(LED_RED);
    } else {
        button_short_pressed = true;
        ESP_LOGI(TAG, "Button short press detected");
        gpio_toggle_leds(LED_GREEN);
    }
}

esp_err_t button_interrupt_init(void) {
    if (button_interrupt_initialized) {
        ESP_LOGW(TAG, "Button interrupt already initialized");
        return ESP_OK;
    }
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), 
        TAG, "Failed to configure GPIO for ISR");


    /* Initialize debounce timer */
    const esp_timer_create_args_t debounce_timer_args = {
        .callback = debounce_timer_callback,
        .name = "debounce_timer"
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&debounce_timer_args, &debounce_timer), 
        TAG, "Failed to create debounce timer");

    /* Install ISR service and add ISR handler */
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), 
        TAG, "Failed to install ISR service");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL), 
        TAG, "Failed to add ISR handler");

    ESP_LOGI(TAG, "Button interrupt initialized");
    button_interrupt_initialized = true;

    return ESP_OK;
}
esp_err_t button_interrupt_enable_wakeup(void)
{
    if (!button_interrupt_initialized) {
        ESP_LOGE(TAG, "Button interrupt not initialized. Init the button first!");
        return ESP_ERR_INVALID_STATE;
    }

    /* Enable deep sleep wake-up from button press */
    ESP_RETURN_ON_ERROR(esp_sleep_enable_gpio_wakeup(), 
        TAG, "Failed to enable GPIO wakeup");

    ESP_RETURN_ON_ERROR(gpio_wakeup_enable(BUTTON_GPIO, GPIO_INTR_LOW_LEVEL), 
        TAG, "Failed to enable GPIO wakeup");

    ESP_RETURN_ON_ERROR(esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_GPIO, ESP_GPIO_WAKEUP_GPIO_LOW), 
        TAG, "Failed to enable deep sleep wakeup");

    return ESP_OK;
}

bool is_button_short_pressed(void) {
    bool pressed = button_short_pressed;
    button_short_pressed = false;
    return pressed;
}

bool is_button_long_pressed(void) {
    bool pressed = button_long_pressed;
    button_long_pressed = false;
    return pressed;
}

void clear_button_press_states(void) {
    button_short_pressed = false;
    button_long_pressed = false;
}

