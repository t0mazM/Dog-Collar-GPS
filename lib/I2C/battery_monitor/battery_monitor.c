#include "battery_monitor.h"

void battery_monitor_read(void){
    uint8_t battery_voltage;
    uint8_t battery_soc;
    uint8_t battery_temp;
    i2c_read_byte(BQ27441_ADDRESS, VOLTAGE_CMD, &battery_voltage);
    i2c_read_byte(BQ27441_ADDRESS, SOC_CMD, &battery_soc);
    i2c_read_byte(BQ27441_ADDRESS, TEMP_CMD, &battery_temp);

    printf("Battery Voltage: %d mV\n", battery_voltage);
    printf("Battery State of Charge: %d %%\n", battery_soc);
    printf("Battery Temperature: %d C\n", battery_temp);
}