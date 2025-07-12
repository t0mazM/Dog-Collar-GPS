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
#define WIFI_MAX_CONNECTION_TIMEOUT_MS 10000

esp_err_t wifi_connect_and_start_services(void);

/**
 * @brief Stops all Wi-Fi services and cleans up resources.
 *
 * This function stops all Wi-Fi related services in order:
 * 1. Unregisters event handlers.
 * 2. Stops the HTTP server.
 * 3. Disconnects from the Wi-Fi network.
 * 4. Stops the Wi-Fi driver.
 * 5. Deinitializes the Wi-Fi driver.
 * 6. Cleans up the event group.
 * 7. Resets the retry counter for the next connection attempt.
 * 8. Logs the final status of the shutdown process.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t wifi_stop_all_services(void);

#endif // WIFI_MANAGER_H