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

    printf("Battery Data:\n");
    printf("Voltage: %.2f V\n", battery_data->voltage);
    printf("State of Charge: %.2f %%\n", battery_data->soc);
    printf("Temperature: %.2f C\n", battery_data->temperature);
}

esp_err_t read_voltage(float *voltage) {
    uint16_t raw;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, VOLTAGE_CMD, &raw);
    if (ret != ESP_OK) {
        return ret;
    }
    *voltage = raw / 1000.0f;
    return ESP_OK;
}

esp_err_t read_soc(float *soc) {
    uint8_t raw;
    esp_err_t ret = i2c_read_8bit(BQ27441_ADDRESS, SOC_CMD, &raw);
    if (ret != ESP_OK) {
        return ret;
    }
    *soc = (float)raw;
    return ESP_OK;
}

esp_err_t read_temperature(float *temperature) {
    uint16_t raw;
    esp_err_t ret = i2c_read_16bit(BQ27441_ADDRESS, TEMP_CMD, &raw);
    if (ret != ESP_OK) {
        return ret;
    }
    *temperature = (raw * 0.1f) - 273.15f;
    return ESP_OK;
}
