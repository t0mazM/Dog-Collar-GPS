#include <stdio.h>
#include <i2c.h>
#include <uart.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    init_i2c();
}