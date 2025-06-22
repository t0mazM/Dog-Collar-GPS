#include "battery_monitor.h"




void battery_monitor_read(void){
uint16_t voltage;
i2c_read_16bit(BQ27441_ADDRESS, VOLTAGE_CMD, &voltage);
printf("Battery Voltage: %.3f V \n", voltage/1000.0);


uint8_t soc;
i2c_read_8bit(BQ27441_ADDRESS, SOC_CMD, &soc);
printf("Battery State of Charge: %d %%\n", soc);

uint16_t temperature;
i2c_read_16bit(BQ27441_ADDRESS, TEMP_CMD, &temperature);
printf("Battery Temperature: %.2f C\n", (temperature * 0.1)-273.15);

}
