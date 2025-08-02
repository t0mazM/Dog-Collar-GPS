#include "gpio_expander.h"

static const char *TAG = "GPIO_EXPANDER";
static uint8_t gpio_output_state = 0xFF; //Global variable to hold the state of the GPIO pins

uint8_t gpio_expander_get_output_state(void) {
    return gpio_output_state;
}
void gpio_expander_update_output_state(uint8_t state) {
    gpio_output_state = state;
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}


// Initialize PCF8574 pins (all LEDs off)
esp_err_t gpio_init(void) {

    // Initialize I2C for GPIO expander
    ESP_RETURN_ON_ERROR(i2c_init(), 
                        TAG, "Failed to initialize I2C for GPIO expander"
    );

    // Set all GPIO pins to high (LEDs off)
    gpio_output_state = 0xFF;
    ESP_RETURN_ON_ERROR(i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state), 
                        TAG, "Failed to set default GPIO expander state"
    );
    
    ESP_LOGI(TAG, "GPIO expander initialized");
    return ESP_OK;
}

esp_err_t gpio_sync_state(void) {
    ESP_RETURN_ON_ERROR(i2c_read_8bit(PCF8574_ADDR, REG_ADDR_NOT_USED, &gpio_output_state),
                        TAG, "Failed to read current GPIO state");
    return ESP_OK;
}

void gpio_turn_on_leds(uint8_t led_mask) {
    gpio_output_state &= ~led_mask; 
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

void gpio_turn_off_leds(uint8_t led_mask) {
    gpio_output_state |= led_mask; 
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

esp_err_t gpio_toggle_leds(uint8_t led_mask) {

    ESP_RETURN_ON_ERROR(gpio_sync_state(), 
                    TAG, "Failed to sync GPIO state before turning on LEDs");

    /* XOR operation flips the bits specified in led_mask */
    gpio_output_state ^= led_mask;

    ESP_RETURN_ON_ERROR(i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state), 
                        TAG, "Failed to toggle GPIO LEDs"
    );
    return ESP_OK;
}
