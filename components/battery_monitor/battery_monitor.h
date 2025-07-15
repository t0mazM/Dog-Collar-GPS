#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <i2c.h>

// I2C Device Addresses
#define BQ27441_ADDRESS 0x55
#define VOLTAGE_CMD     0x04
#define SOC_CMD         0x1C
#define TEMP_CMD        0x02
#define FLAGS_CMD       0x06

typedef struct {
    uint8_t i2c_address; // Default I2C address for BQ27441
    float voltage; //In mili Volts
    float soc;  //In %
    float temperature; //In degrees Celsius
    uint16_t flags; // Battery flags
} battery_data_t;
extern battery_data_t battery_data;


esp_err_t battery_monitor_init(void);
void battery_monitor_update_battery_data(battery_data_t *battery_data);

esp_err_t read_voltage(float *voltage);
esp_err_t read_soc(float *soc);
esp_err_t read_temperature(float *temp);
esp_err_t read_flags(uint16_t *flags);


#endif  // BATTERY_MONITOR_H