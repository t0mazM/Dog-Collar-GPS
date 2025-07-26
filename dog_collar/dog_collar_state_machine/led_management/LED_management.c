#include "LED_management.h"

/* 
This module is responsible for managing the LEDS
So the blinking and other visual indications will be handled here.

TODO: 
- Implement LED blinking for difrent states:
- Implement LED pattern for charging
*/

static volatile dog_collar_state_t led_current_state = DOG_COLLAR_STATE_NORMAL;

void led_management_set_pattern(dog_collar_state_t state) {
    led_current_state = state;
}

void led_task(void *pvParameters) {
    for (;;) {
        switch (led_current_state) {
            case DOG_COLLAR_STATE_CHARGING:
                led_set_charging_pattern();
                break;
            case DOG_COLLAR_STATE_LOW_BATTERY:
                led_blink_low_battery();
                break;
            case DOG_COLLAR_STATE_ERROR:
                led_blink_error();
                break;
            default:
                led_blink_normal();
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust as needed
    }
}
static void led_set_charging_pattern(){

    gpio_set_leds(LED_GREEN); // Example: turn on green LED
    vTaskDelay(pdMS_TO_TICKS(500)); // Adjust delay as needed
    gpio_clear_leds(LED_GREEN);

}


void led_blink_normal() {
    
    gpio_turn_on_leds(LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(500));
}