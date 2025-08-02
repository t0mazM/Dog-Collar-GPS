/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */


#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <i2c.h>

// I2C Device Addresses
#define BQ27441_ADDRESS 0x55
#define VOLTAGE_CMD     0x04
#define CURRENT_CMD     0x10
#define POWER_CMD       0x18
#define SOC_CMD         0x1C
#define TEMP_CMD        0x02
#define FLAGS_CMD       0x06

#define BAT_MON_LOG_BUF_SIZE 512

typedef struct {
    uint8_t i2c_address;    // Default I2C address for BQ27441
    float voltage;          // In mili Volts
    int16_t current;        // In mili Amperes
    float soc;              // In %
    float temperature;      // In degrees Celsius
    uint16_t flags;         // In binary format
} battery_data_t;
extern battery_data_t battery_data;

typedef struct {
    bool over_temp;
    bool under_temp;
    bool full_charge;
    bool charging;
    bool battery_detected;
    bool soc1;
    bool socf;
    bool discharging;
} battery_status_flags_t;
extern battery_status_flags_t battery_status_flags;

/**
 * @brief Initializes the battery monitor
 * 
 * It initializes the I2C interface for the battery monitor 
 * and sets initial values for the battery data to zero.
 * 
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t battery_monitor_init(void);

/**
 * @brief Reads the battery data and updates the values in the battery_data struct
 * @param battery_data Pointer to the battery_data_t struct to be updated
 * 
 * This function reads:
 *  - Battery voltage
 *  - Battery state of charge
 *  - Battery temperature
 *  - Battery flags
 * @note If any read fails, it logs a warning and sets the corresponding value to an invalid state.
 * * @return None
 */
esp_err_t battery_monitor_update_battery_data(void);

/**
 * @brief Returns the string with data from battery monitor
 * 
 * @param string_buffer Pointer to the buffer to store the string
 * @param string_buffer_size Size of the string buffer
 * @return ESP_OK on success, or an error code on failure
 */
int battery_monitor_get_data_string(char *string_buffer, size_t string_buffer_size);

/**
 * @brief Logs the battery data
 * 
 * This function retrieves the battery data as a string and logs it.
 */
#endif  // BATTERY_MONITOR_H