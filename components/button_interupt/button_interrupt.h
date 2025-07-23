#ifndef BUTTON_INTERRUPT_H
#define BUTTON_INTERRUPT_H


#ifndef IRAM_ATTR
#define IRAM_ATTR /* Defines the IRAM attribute for interrupt handler */
#endif


#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define BUTTON_GPIO         9    
#define DEBOUNCE_TIME_MS    50
#define LONG_PRESS_TIME_MS  2000


esp_err_t button_interrupt_init(void);

#endif // BUTTON_INTERRUPT_H