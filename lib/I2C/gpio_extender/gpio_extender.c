#include "gpio_extender.h"

static const char *TAG = "GPIO_EXTENDER";
static uint8_t gpio_output_state = 0xFF; //Global variable to hold the state of the GPIO pins

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

esp_err_t gpio_read_inputs(uint8_t *input_state) {
    ESP_RETURN_ON_ERROR(i2c_read_8bit(PCF8574_ADDR, REG_ADDR_NOT_USED, input_state), TAG, "Failed to read GPIO inputs");
    return ESP_OK;
}

esp_err_t gps_force_on_set(bool enable){
    //Calculcate the new state
    uint8_t new_state = enable
                        ? (gpio_output_state |  GPS_FORCE_ON)  /* set bit */
                        : (gpio_output_state & ~GPS_FORCE_ON); /* clear bit */

    // If bit is alredy in the required state, do nothing and return
    if (new_state == gpio_output_state) {
        return ESP_OK;
    }

    // Send new state to the IO extender
    ESP_RETURN_ON_ERROR(i2c_write_byte(PCF8574_ADDR, REG_ADDR_NOT_USED, new_state), TAG, "Failed to set FORCE_ON pin state");

    // Add the new state to the global variable only if the write was successful
    gpio_output_state = new_state;
    return ESP_OK;
}