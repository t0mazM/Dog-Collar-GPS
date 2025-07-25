#include "gpio_expander.h"

static const char *TAG = "GPIO_EXPANDER";
static uint8_t gpio_output_state = 0xFF; //Global variable to hold the state of the GPIO pins

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

//TODO add this function to gps_l96 module
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