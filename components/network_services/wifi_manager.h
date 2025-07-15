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
#include "nvs_flash.h"
#include "file_system_littlefs/file_system_littlefs.h"
#include "mdns_service.h"
#include "network_services/http_server.h"

#define WIFI_RECONNECT_RETRIES_NUM 10
#define WIFI_MAX_CONNECTION_TIMEOUT_MS 10000


/**
 * @brief Initializes all modules for WI-FI connectivity 
 *
 * This function initializes the NVS flash storage, connects to the specified Wi-Fi network,
 * sets up the mDNS service, and starts the HTTP server.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t wifi_init(void);

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

/**
 * @brief Retries stopping all Wi-Fi services
 *
 * This function attempts to stop all Wi-Fi services, retrying up to _max_retry_count times
 * Each failed attempt is logged.
 *
 * @param _max_retry_count The maximum number of retry attempts.
 * @return esp_err_t ESP_OK if successful, or an error code if all retries fail.
 */
esp_err_t wifi_stop_all_services_retry(uint16_t _max_retry_count);

#endif // WIFI_MANAGER_H