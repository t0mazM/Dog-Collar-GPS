#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

// Pin configuration
#define I2C_SDA  GPIO_NUM_6
#define I2C_SCL  GPIO_NUM_7

// Config constants
#define I2C_FREQ_HZ 100000
#define WAIT_TIME   (1000 / portTICK_PERIOD_MS)

// Error handling macro
#define RETURN_ON_ERROR_I2C(err, tag, msg, cmd) \
    do { \
        if ((err) != ESP_OK) { \
            ESP_LOGE(tag, "%s: %s", msg, esp_err_to_name(err)); \
            i2c_cmd_link_delete(cmd); \
            return err; \
        } \
    } while (0)

// Function declarations
esp_err_t i2c_init(void);
esp_err_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
esp_err_t i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);

#endif  // I2C_H
