/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../dog_collar/dog_collar_state_machine/dog_collar_state_machine.h"
#include "../dog_collar/dog_collar_state_machine/led_management/LED_management.h"

void app_main() {

    vTaskDelay(6000 / portTICK_PERIOD_MS); // Delay to allow system to stabilize

    /* Create FreeRTOS tasks */
    xTaskCreate(state_machine_task, "state_machine_task", 4096, NULL, 1, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 2, NULL);

    /* Delete the main task */
    vTaskDelete(NULL);
}