#ifndef ERR_HANDLING_H
#define ERR_HANDLING_H
#define I2C_COM_H

#include "esp_log.h"
#include "esp_err.h"


//TODO1: Update app state to halt/stop. Go into unrecoverable ERROR state. 
//TODO2: Based on TAG try to recover from the error, if possible.
#define CUSTUM_ERROR_CHECK(ESP_ERR) do { \
    esp_err_t err_rc = (ESP_ERR); \
    if (err_rc != ESP_OK) { \
        ESP_LOGE(TAG, "HUGE ERROR %s at %s:%d", esp_err_to_name(err_rc), __FILE__, __LINE__); \
    } \
} while(0)

#endif  // ERR_HANDLING_H