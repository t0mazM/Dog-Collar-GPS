#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <i2c.h>

// I2C Device Addresses
#define PCF_ADDR        0x27
#define BQ27441_ADDRESS 0x55
#define VOLTAGE_CMD     0x04
#define SOC_CMD         0x1C
#define TEMP_CMD        0x02

void battery_monitor_read(void);





#endif  // BATTERY_MONITOR_H