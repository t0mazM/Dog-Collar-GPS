#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h" 
#include "esp_check.h"
#include "esp_log.h"
#include "file_system_littlefs.h"
#include "mdns_service.h"
#include "http_server.h"

#define WIFI_RECONNECT_RETRIES_NUM 10

void wifi_init_sta(void);

#endif // WIFI_MANAGER_H