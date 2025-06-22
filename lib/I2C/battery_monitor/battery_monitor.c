#include "battery_monitor.h"

void battery_monitor_read(void){
uint8_t buffer[2];
i2c_read_bytes(BQ27441_ADDRESS, VOLTAGE_CMD, buffer, 2);
uint16_t voltage = buffer[0] | (buffer[1] << 8);
printf("Battery Voltage: %.3f V \n", voltage/1000.0);

i2c_read_bytes(BQ27441_ADDRESS, SOC_CMD, buffer, 2);
uint16_t soc = buffer[0] | (buffer[1] << 8);
printf("Battery State of Charge: %.2f %%\n", soc/100.0);

i2c_read_bytes(BQ27441_ADDRESS, TEMP_CMD, buffer, 2);
int16_t temperature = (buffer[0] | (buffer[1] << 8)) / 10;
printf("Battery Temperature: %.1f C\n", temperature / 10.0);
}