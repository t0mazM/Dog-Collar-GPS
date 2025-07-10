#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "esp_http_server.h"
#include "esp_err.h" 
#include "esp_check.h"
#include "esp_log.h"
#include "file_system_littlefs.h"

#define WIFI_RECONNECT_RETRIES_NUM 10

void wifi_init_sta(void);
void start_mdns_service(void);
void start_http_server(void);

#endif // WIFI_H