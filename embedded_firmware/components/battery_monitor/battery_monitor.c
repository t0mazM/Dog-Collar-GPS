/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */


#include "battery_monitor.h"

static const char *TAG = "BATTERY_MONITOR";
battery_data_t battery_data = {0};
battery_status_flags_t battery_status_flags = {0};

/* Declarations of static functions used for reading battery data */
static esp_err_t read_voltage(float *voltage);
static esp_err_t read_soc(uint8_t *soc);
static esp_err_t read_temperature(float *temp);
static esp_err_t read_flags(uint16_t *flags);
static esp_err_t read_current(int16_t *current);
static void battery_monitor_parse_flags(void);
static void battery_monitor_log_data(void);

esp_err_t battery_monitor_init(void) {

    // Initialize I2C for battery monitor
    ESP_RETURN_ON_ERROR(i2c_init(), 
                        TAG, "Failed to initialize I2C for battery monitor"
    );

    battery_data.i2c_address = BQ27441_ADDRESS;
    battery_data.voltage = 0.0f;
    battery_data.current = 0;
    battery_data.soc = 0;
    battery_data.temperature = 0.0f;
    battery_data.flags = 0x0000;

    return ESP_OK;
}

esp_err_t battery_monitor_update_battery_data(void) {

    /* Temporary variables for battery data */
    float voltage;
    int16_t current;
    uint8_t soc;
    float temperature;
    uint16_t flags;

    esp_err_t overall_status = ESP_OK;

    /* Read battery voltage */
    if (read_voltage(&voltage) == ESP_OK) {
        battery_data.voltage = voltage;
    } else {
        ESP_LOGW(TAG, "Failed to read battery voltage");
        battery_data.voltage = -1.0f;
        overall_status = ESP_FAIL; 
    }

    /* Read battery current */
    if( read_current(&current) == ESP_OK) {
        battery_data.current = current;
    } else {
        ESP_LOGW(TAG, "Failed to read battery current");
        battery_data.current = -1.0f;
        overall_status = ESP_FAIL;
    }

    /* Read battery state of charge */
    if (read_soc(&soc) == ESP_OK) {
        battery_data.soc = soc;
    } else {
        ESP_LOGW(TAG, "Failed to read battery state of charge");
        battery_data.soc = -1.0f;
        overall_status = ESP_FAIL;
    }

    /* Read battery temperature */
    if (read_temperature(&temperature) == ESP_OK) {
        battery_data.temperature = temperature;
    } else {
        ESP_LOGW(TAG, "Failed to read battery temperature");
        battery_data.temperature = -1.0f;
        overall_status = ESP_FAIL;
    }

    /* Read battery flags */
    if (read_flags(&flags) == ESP_OK) {
        battery_data.flags = flags;
        battery_monitor_parse_flags(); // Parse the flags into bool status structure for easier access
    } else {
        ESP_LOGW(TAG, "Failed to read battery flags");
        battery_data.flags = 0xFFFF;
        overall_status = ESP_FAIL;
    }

    /* Log the battery data */
    //battery_monitor_log_data();

    if (overall_status != ESP_OK) {
        ESP_LOGE(TAG, "Battery data update encountered errors");
    } else {
        // ESP_LOGI(TAG, "Battery data updated successfully");
    }
    return overall_status;
}

static esp_err_t read_voltage(float *voltage) {
    uint16_t raw_voltage;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, VOLTAGE_CMD, &raw_voltage);
    if (ret != ESP_OK) {
        return ret;
    }
    *voltage = raw_voltage / 1000.0f; // Convert to volts
    return ESP_OK;
}

static esp_err_t read_soc(uint8_t *soc) {
    return(i2c_read_8bit(BQ27441_ADDRESS, SOC_CMD, soc));
}

static esp_err_t read_temperature(float *temperature) {
    uint16_t raw_temperature;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, TEMP_CMD, &raw_temperature);
    if (ret != ESP_OK) {
        return ret;
    }
    *temperature = (raw_temperature * 0.1f) - 273.15f; // Convert to Celsius
    return ESP_OK;
}

static esp_err_t read_flags(uint16_t *flags) {
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, FLAGS_CMD, flags);
    if (ret != ESP_OK){ 
        return ret;
    }
    return ESP_OK;
}

static esp_err_t read_current(int16_t *current) {
    return (i2c_read_16bit(BQ27441_ADDRESS, CURRENT_CMD, (uint16_t *)current));
}
static void battery_monitor_parse_flags(void) {
 
    battery_status_flags.over_temp        = (battery_data.flags >> 15) & 0x01;
    battery_status_flags.under_temp       = (battery_data.flags >> 14) & 0x01;  
    battery_status_flags.full_charge      = (battery_data.flags >> 9)  & 0x01; 
    battery_status_flags.charging         = (battery_data.flags >> 8)  & 0x01;  
    battery_status_flags.battery_detected = (battery_data.flags >> 3)  & 0x01;  
    battery_status_flags.soc1             = (battery_data.flags >> 2)  & 0x01; 
    battery_status_flags.socf             = (battery_data.flags >> 1)  & 0x01;  
    battery_status_flags.discharging      = (battery_data.flags >> 0)  & 0x01;  

}

int battery_monitor_get_data_string(char *string_buffer, size_t string_buffer_size) {
    int written = snprintf(string_buffer, string_buffer_size,
        "\n============= Battery Monitor Data ==============\n"
        "Battery Data:\n"
        "Voltage: %.2f V\n"
        "Current: %d mA\n"
        "State of Charge: %.2f %%\n"
        "Temperature: %.2f °C\n"
        "Raw Flags: 0x%04X\n"
        "Battery Status Flags:\n"
        "  Over Temperature: %s\n"
        "  Under Temperature: %s\n"
        "  Full Charge: %s\n"
        "  Charging: %s\n"
        "  Battery Detected: %s\n"
        "  SOC1: %s\n"
        "  SOCF: %s\n"
        "  Discharging: %s\n"
        "\n=================================================\n",
        battery_data.voltage,
        (int)battery_data.current,
        battery_data.soc,
        battery_data.temperature,
        battery_data.flags,
        battery_status_flags.over_temp ? "Yes" : "No",
        battery_status_flags.under_temp ? "Yes" : "No",
        battery_status_flags.full_charge ? "Yes" : "No",
        battery_status_flags.charging ? "Yes" : "No",
        battery_status_flags.battery_detected ? "Yes" : "No",
        battery_status_flags.soc1 ? "Yes" : "No",
        battery_status_flags.socf ? "Yes" : "No",
        battery_status_flags.discharging ? "Yes" : "No"
    );

    if (written < 0 || (size_t)written >= string_buffer_size) {
        ESP_LOGW(TAG, "Buffer too small for battery data string");
        return -1;  // Indicate error
    }
    return written;  // Return the number of bytes written
}

static void battery_monitor_log_data(void) {
    char log_buffer[BAT_MON_LOG_BUF_SIZE];
    if (battery_monitor_get_data_string(log_buffer, sizeof(log_buffer)) > 0) {
        ESP_LOGI(TAG, "%s", log_buffer);
    }
    else {
        ESP_LOGE(TAG, "Failed to get battery data string");
    }
}