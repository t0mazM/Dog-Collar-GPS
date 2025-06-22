#include "battery_monitor.h"

battery_data_t battery_data = {
    .i2c_address = BQ27441_ADDRESS,
    .voltage = 0.0f,
    .soc = 0.0f,
    .temperature = 0.0f
};

void battery_monitor_update_battery_data(void){

    read_temperature(&battery_data.temperature);
    read_voltage(&battery_data.voltage);
    read_soc(&battery_data.soc);

    printf("Battery Data:\n");
    printf("Voltage: %.2f V\n", battery_data.voltage);
    printf("State of Charge: %.2f %%\n", battery_data.soc);
    printf("Temperature: %.2f C\n", battery_data.temperature);
}

esp_err_t read_voltage(float *voltage) {
    uint16_t raw;
    if (i2c_read_16bit(battery_data.i2c_address, VOLTAGE_CMD, &raw) != ESP_OK) return ESP_FAIL;
    *voltage = raw / 1000.0f;
    return ESP_OK;
}

esp_err_t read_soc(float *soc) {
    uint8_t raw;
    if (i2c_read_8bit(battery_data.i2c_address, SOC_CMD, &raw) != ESP_OK) return ESP_FAIL;
    *soc = raw;
    return ESP_OK;
}

esp_err_t read_temperature(float *temp) {
    uint16_t raw;
    if (i2c_read_16bit(battery_data.i2c_address, TEMP_CMD, &raw) != ESP_OK) return ESP_FAIL;
    *temp = (raw * 0.1f) - 273.15f;
    return ESP_OK;
}