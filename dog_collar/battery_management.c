#include "battery_management.h"

static const char *TAG = "BATTERY_MANAGEMENT";

/*TODO: Implement battery management functions. Add a state machine where
    - You check the battery voltage and state of charge. If critical low levels are detected,
    you can trigger a low power mode for ESP32 na GPS module. Periodicly show red blinking to indicate low battery
    - If the battery is charging, you can show a different LED pattern - using 3 leds you can show charging status, with linkin also
    - If the battery is fully charged, you can show a green LED pattern
*/