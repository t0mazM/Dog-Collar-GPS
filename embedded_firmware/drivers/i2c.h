/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

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

#define I2C_MUTEX_TIMEOUT_MS 200

#define REG_ADDR_NOT_USED -1

// Config constants
#define I2C_FREQ_HZ 100000
#define WAIT_TIME   (1000 / portTICK_PERIOD_MS)

// Error handling macro
#define RETURN_ON_ERROR_I2C(err, tag, msg, cmd)\
    do { \
        if ((err) != ESP_OK) { \
            ESP_LOGE(tag, "%s: %s", msg, esp_err_to_name(err));\
            i2c_cmd_link_delete(cmd);\
            xSemaphoreGive(i2c_mutex);\
            return err; \
        } \
    } while (0)

/**
 * @brief Initialize I2C and create a mutex for I2C operations.
 *
 * This function initializes the I2C driver and creates a mutex to protect
 * I2C operations from concurrent access by multiple tasks.
 *
 * @return esp_err_t
 */
esp_err_t i2c_init(void);

/**
 * @brief Write a byte to an I2C device.
 * This function writes a byte to a specified register of an I2C device.
 * Handles I2C mutex to prevent simultaneous access by multiple tasks.
 *
 * @param dev_addr Device address
 * @param reg_addr Register address
 * @param data Data byte to write
 * @return esp_err_t
 */
esp_err_t i2c_write_byte(uint8_t dev_addr, int8_t reg_addr, uint8_t data);

/**
 * @brief Read a 16-bit value from an I2C device.
 * This function reads a 16-bit value from a specified register of an I2C device.
 * Handles I2C mutex to prevent simultaneous access by multiple tasks.
 *
 * @param dev_addr Device address
 * @param reg_addr Register address
 * @param data Pointer to store the read 16-bit value
 * @return esp_err_t
 */
esp_err_t i2c_read_16bit(uint8_t dev_addr, int8_t reg_addr, uint16_t *data);

/**
 * @brief Read an 8-bit value from an I2C device.
 * This function reads an 8-bit value from a specified register of an I2C device
 * Handles I2C mutex to prevent simultaneous access by multiple tasks.
 * 
 * @param dev_addr Device address
 * @param reg_addr Register address
 * @param data Pointer to store the read 8-bit value
 * @return esp_err_t
 */
esp_err_t i2c_read_8bit(uint8_t dev_addr, int8_t reg_addr, uint8_t *data);

/**
 * @brief Combine two bytes into a 16-bit value.
 *
 * This function combines a low byte and a high byte into a single 16-bit value.
 *
 * @param low Low byte
 * @param high High byte
 * @return uint16_t Combined 16-bit value
 */
uint16_t combine_bytes(uint8_t low, uint8_t high);



#endif  // I2C_H
