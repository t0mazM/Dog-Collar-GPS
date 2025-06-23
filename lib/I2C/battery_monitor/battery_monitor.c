#include "battery_monitor.h"

static const char *TAG = "BATTERY_MONITOR";


battery_data_t battery_data = {
    .i2c_address = BQ27441_ADDRESS,
    .voltage = 0.0f,
    .soc = 0.0f,
    .temperature = 0.0f
};

void battery_monitor_update_battery_data(battery_data_t *battery_data) {

    float temp;
    
    if (read_voltage(&temp) == ESP_OK) {
        battery_data->voltage = temp;
    } else {
        ESP_LOGW(TAG, "Failed to read voltage");
    }

    if (read_soc(&temp) == ESP_OK) {
        battery_data->soc = temp;
    } else {
        ESP_LOGW(TAG, "Failed to read state of charge");
    }

    if (read_temperature(&temp) == ESP_OK) {
        battery_data->temperature = temp;
    } else {
        ESP_LOGW(TAG, "Failed to read temperature");
    }

ESP_LOGI(TAG, "Battery Voltage: %.2f V, SOC: %.2f %%, Temperature: %.2f C", battery_data->voltage, battery_data->soc, battery_data->temperature);
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

