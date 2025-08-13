/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <spi_flash_mmap.h>
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

#define WIFI_MAX_CONNECTION_TIMEOUT_MS 1 * 60 * 1000 // 1 minute

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

/**
 * @brief Reconnects to the Wi-Fi network.
 *
 * This function stops all services, reinitializes the Wi-Fi, and attempts to reconnect.

 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t wifi_manager_reconnect(void);

/**
 * @brief Checks if the Wi-Fi manager is initialized and currently connected to a network.
 *
 * This function verifies that the Wi-Fi event group has been created and checks the connection status
 * by testing the WIFI_CONNECTED_BIT. Returns true if connected, false otherwise.
 *
 * @return true if Wi-Fi is initialized and connected, false otherwise.
 */
bool wifi_manager_is_initialized_and_connected(void);



#endif // WIFI_MANAGER_H