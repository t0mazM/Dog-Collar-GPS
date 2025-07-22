#ifndef BUTTON_INTERRUPT_H
#define BUTTON_INTERRUPT_H

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define BUTTON_GPIO         0     
#define BUTTON_HOLD_TIME_MS 2000  
#define BUTTON_DEBOUNCE_MS  50

#endif // BUTTON_INTERRUPT_H