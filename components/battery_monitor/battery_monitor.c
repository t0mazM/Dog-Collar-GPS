#include "battery_monitor.h"

static const char *TAG = "BATTERY_MONITOR";

battery_data_t battery_data = {0};

esp_err_t battery_monitor_init(void) {

    // Initialize I2C for battery monitor
    ESP_RETURN_ON_ERROR(i2c_init(), 
                        TAG, "Failed to initialize I2C for battery monitor"
    );

    battery_data.i2c_address = BQ27441_ADDRESS;
    battery_data.voltage = 0.0f;
    battery_data.soc = 0.0f;
    battery_data.temperature = 0.0f;
    battery_data.flags = 0x0000;

    return ESP_OK;
}

void battery_monitor_update_battery_data(battery_data_t *battery_data) {

    /* Temporary variables for battery data */
    float temp_voltage;
    float temp_soc;
    float temp_temperature;
    uint16_t flags;

    /* Read battery voltage */
    if (read_voltage(&temp_voltage) == ESP_OK) {
        battery_data->voltage = temp_voltage;
    } else {
        ESP_LOGW(TAG, "Failed to read battery voltage");
        battery_data->voltage = -1.0f;
    }

    /* Read battery state of charge */
    if (read_soc(&temp_soc) == ESP_OK) {
        battery_data->soc = temp_soc;
    } else {
        ESP_LOGW(TAG, "Failed to read battery state of charge");
        battery_data->soc = -1.0f;
    }

    /* Read battery temperature */
    if (read_temperature(&temp_temperature) == ESP_OK) {
        battery_data->temperature = temp_temperature;
    } else {
        ESP_LOGW(TAG, "Failed to read battery temperature");
        battery_data->temperature = -1.0f;
    }

    /* Read battery flags */
    if (read_flags(&flags) == ESP_OK) {
        battery_data->flags = flags;
    } else {
        ESP_LOGW(TAG, "Failed to read battery flags");
        battery_data->flags = 0xFFFF; 
    }
}

esp_err_t read_voltage(float *voltage) {
    uint16_t raw_voltage;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, VOLTAGE_CMD, &raw_voltage);
    if (ret != ESP_OK) {
        return ret;
    }
    *voltage = raw_voltage / 1000.0f; // Convert to volts
    return ESP_OK;
}

esp_err_t read_soc(float *soc) {
    uint8_t raw_soc;
    esp_err_t ret = i2c_read_8bit(BQ27441_ADDRESS, SOC_CMD, &raw_soc);
    if (ret != ESP_OK) {
        return ret;
    }
    *soc = (float)raw_soc; // State of Charge is already in percentage
    return ESP_OK;
}

esp_err_t read_temperature(float *temperature) {
    uint16_t raw_tempearture;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, TEMP_CMD, &raw_tempearture);
    if (ret != ESP_OK) {
        return ret;
    }
    *temperature = (raw_tempearture * 0.1f) - 273.15f; // Convert to Celsius
    return ESP_OK;
}

esp_err_t read_flags(uint16_t *flags) {
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, FLAGS_CMD, flags);
    if (ret != ESP_OK) {
        return ret;
    }
    printf("Battery flags: 0x%04X\n", *flags);  // prints hex flags
    return ESP_OK;
}