#include "gpio_extender.h"

static const char *TAG = "GPIO_EXTENDER";
static uint8_t gpio_output_state = 0xFF; // All LEDs off initially (bits all high)

// Initialize PCF8574 pins (all LEDs off)
void gpio_init(void) {
    gpio_output_state = 0xFF;
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

void gpio_turn_on_leds(uint8_t led_mask) {
    gpio_output_state &= ~led_mask; 
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

void gpio_turn_off_leds(uint8_t led_mask) {
    gpio_output_state |= led_mask; 
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
}

void gpio_reset_gps(void) {
    // Reset GPS by setting pin to 0 for 100 ms
    gpio_output_state &= ~GPS_RESET; // Set pin to 0
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
    vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100 ms
    gpio_output_state |= GPS_RESET; // Set pin back to 1
    i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state);
    vTaskDelay(1000/ portTICK_PERIOD_MS); // Wait for 1 second to allow GPS to reset - some commands were not set becouse module was not fully on
    ESP_LOGI(TAG, "GPS reset done");
}

void gpio_read_inputs(void) {
    uint8_t input_state;
    i2c_read_8bit(PCF8574_ADDR, REG_ADDR_NOT_USED, &input_state);
    //TODO: store this bool into struct in the GPS module file
    bool geo_fence_triggered = (input_state & GEO_FENCE) != 0; // false if bit is 1, true if bit is 0
}

esp_err_t gpio_set_pin_force(bool on) {
    if (on) {
        
        // Turn OFF: set the bit to set pin HIGH
        gpio_output_state |= GPS_FORCE_ON;
        
    } else {
        // Turn ON: clear the bit to set pin LOW (active)
        gpio_output_state &= ~GPS_FORCE_ON;
    }
    ESP_RETURN_ON_ERROR(
        i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, gpio_output_state),
        TAG,
        "Failed to set FORCE_ON pin"
    );
    return ESP_OK;
}