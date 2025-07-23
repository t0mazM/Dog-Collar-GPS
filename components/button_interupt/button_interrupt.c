#include "button_interrupt.h"


static const char *TAG = "BUTTON_HANDLER";
static bool button_interrupt_initialized = false;

volatile static bool button_short_pressed = false;
volatile static bool button_long_pressed = false;

static int64_t button_press_time = 0;
static esp_timer_handle_t debounce_timer;

static void IRAM_ATTR button_isr_handler(void* arg) {
    esp_timer_start_once(debounce_timer, DEBOUNCE_TIME_MS * 1000);
}

static void debounce_timer_callback(void* arg) {
    int level = gpio_get_level(BUTTON_GPIO);
    int64_t now = esp_timer_get_time() / 1000; // ms

    if (level == 0) {
        button_press_time = now; // Button pressed
    } else {
        if ((now - button_press_time) >= LONG_PRESS_TIME_MS) {
            button_long_pressed = true;
            ESP_LOGI(TAG, "Button long press detected");
            gpio_toggle_leds(LED_YELLOW);
        } else {
            button_short_pressed = true;
            ESP_LOGI(TAG, "Button short press detected");
            gpio_toggle_leds(LED_RED);
        }
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
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Debounce timer
    const esp_timer_create_args_t debounce_timer_args = {
        .callback = debounce_timer_callback,
        .name = "debounce_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&debounce_timer_args, &debounce_timer));

    // Install ISR service and add ISR handler
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL));

    ESP_LOGI(TAG, "Button interrupt initialized");

    button_interrupt_initialized = true;
    return ESP_OK;
}

bool is_button_short_pressed(void) {
    if (button_short_pressed) {
        button_short_pressed = false;
        return true;
    }
    return false;
}

bool is_button_long_pressed(void) {
    if (button_long_pressed) {
        button_long_pressed = false;
        return true;
    }
    return false;
}

void clear_button_press_states(void) {
    button_short_pressed = false;
    button_long_pressed = false;
}