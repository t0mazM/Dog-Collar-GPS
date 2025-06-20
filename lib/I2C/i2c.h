#ifndef I2C_COM_H
#define I2C_COM_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"



#define I2C_SCL  4          // GPIO for SCL
#define I2C_SDA  5          // GPIO for SDA

#define I2C_FREQ_HZ 100000           // I2C master clock frequency
#define WAIT_TIME 1000 / portTICK_PERIOD_MS

// Custum macro for I2C so we can delete cmd when fail happens (to void memory leak)
#define RETURN_ON_ERROR_I2C(error, tag, message, cmd_to_delete) \
    do { \
        if (error != ESP_OK) { \
            ESP_LOGE(tag, "%s: %s", message, esp_err_to_name(error)); \
            i2c_cmd_link_delete(cmd_to_delete); \
            return error; \
        } \
    } while (0)

typedef struct {
    uint8_t number;        // I2C port number
    uint8_t sda_io_num;    // GPIO for SDA
    uint8_t scl_io_num;   // GPIO for SCL
} I2CPORT;


esp_err_t init_i2c_port(I2CPORT* port);
void init_i2c(void);
esp_err_t write_to_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t data);
esp_err_t read_from_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t* data);





#endif  // I2C_COM_H