#include "LED_management.h"

/* 
This module is responsible for managing the LEDS
So the blinking and other visual indications will be handled here.

TODO: 
 Make the led patterns non-blocking, so the led task can run in parallel with the state machine task
*/

static const char *TAG = "LED_MANAGEMENT";

static volatile dog_collar_state_t led_current_state = DOG_COLLAR_STATE_NORMAL;

static void initializing_led_pattern();
static void normal_led_pattern();
static void low_battery_led_pattern();
static void critical_low_battery_led_pattern();
static void charging_led_pattern();
static void gps_acquiring_led_pattern();
static void gps_ready_led_pattern();
static void gps_file_creation_led_pattern();
static void gps_waiting_for_fix_led_pattern();
static void gps_tracking_led_pattern();
static void gps_paused_led_pattern();
static void wifi_sync_led_pattern();
static void error_led_pattern();



void led_management_set_pattern(dog_collar_state_t state) {
    led_current_state = state;
}

void led_task(void *pvParameters) {
    esp_err_t ret = i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C");
        vTaskDelete(NULL); // Terminate this task
    }
    for (;;) {
        switch (led_current_state) {
            case DOG_COLLAR_STATE_INITIALIZING:
                initializing_led_pattern();
                break;
            case DOG_COLLAR_STATE_NORMAL:
                normal_led_pattern();
                break;
            case DOG_COLLAR_STATE_LOW_BATTERY:
                low_battery_led_pattern();
                break;
            case DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY:
                critical_low_battery_led_pattern();
                break;
            case DOG_COLLAR_STATE_CHARGING:
                charging_led_pattern();
                break;
            case DOG_COLLAR_STATE_GPS_ACQUIRING:
                gps_acquiring_led_pattern();
                break;
            case DOG_COLLAR_STATE_GPS_READY:
                gps_ready_led_pattern();
                break;
            case DOG_COLLAR_STATE_GPS_FILE_CREATION:
                gps_file_creation_led_pattern();
                break;
            case DOG_COLLAR_STATE_WAITING_FOR_GPS_FIX:
                gps_waiting_for_fix_led_pattern();
                break;
            case DOG_COLLAR_STATE_GPS_TRACKING:
                gps_tracking_led_pattern();
                break;
            case DOG_COLLAR_STATE_GPS_PAUSED:
                gps_paused_led_pattern();
                break;
            case DOG_COLLAR_STATE_WIFI_SYNC:
                wifi_sync_led_pattern();
                break;
            case DOG_COLLAR_STATE_ERROR:
                error_led_pattern();
                break;
            default:
                ESP_LOGE("LED_TASK", "Unknown state: %d", led_current_state);
                vTaskDelay(pdMS_TO_TICKS(1000)); 
                break;
        }
  
        vTaskDelay(pdMS_TO_TICKS(50)); // Delay to prevent I2C from being blocked all the time by the led task
    }
}

static void initializing_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void normal_led_pattern() {

    gpio_turn_on_leds(LED_GREEN | LED_YELLOW | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_turn_off_leds(LED_GREEN | LED_YELLOW | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void low_battery_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(2000));
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void critical_low_battery_led_pattern() {
    gpio_turn_on_leds(LED_RED);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(2000));
}
static void charging_led_pattern() {
    gpio_turn_on_leds(LED_GREEN | LED_YELLOW | LED_RED);
    // todo check the state off charging and blink the leds accordingly
}

static void gps_acquiring_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
}

static void gps_ready_led_pattern() {
    gpio_turn_on_leds(LED_GREEN);
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void gps_file_creation_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void gps_waiting_for_fix_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_YELLOW | LED_GREEN | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void gps_tracking_led_pattern() {
    gpio_turn_on_leds(LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_off_leds(LED_GREEN | LED_YELLOW | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(500));
}

static void gps_paused_led_pattern() {
    gpio_turn_on_leds(LED_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(2000));
    gpio_turn_off_leds(LED_YELLOW | LED_RED | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void wifi_sync_led_pattern() {
    gpio_turn_on_leds(LED_GREEN);
    gpio_turn_off_leds(LED_YELLOW | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_turn_on_leds(LED_YELLOW);
    gpio_turn_off_leds(LED_GREEN | LED_RED);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void error_led_pattern() {
    gpio_turn_on_leds(LED_RED);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_turn_off_leds(LED_RED | LED_YELLOW | LED_GREEN);
    vTaskDelay(pdMS_TO_TICKS(500));
}