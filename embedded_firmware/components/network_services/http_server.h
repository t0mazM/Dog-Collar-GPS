#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"

#include "file_system_littlefs/file_system_littlefs.h"
#include "../../dog_collar/dog_collar_state_machine/components_init/components_init.h"

#define RESPONSE_BUFFER_SIZE 4096
#define HTTP_SERVER_PORT_NUM 80
#define CHUNK_BUFFER_SIZE 1460 // TCP MSS (Maximum Segment Size) for ESP32


/**
 * Start the HTTP server.
 * 
 * It initializes the HTTP server and registers URI handlers:
 * - `/` for a simple hello page
 * - `/files` to list files in the filesystem
 * - `/download` to download files from the filesystem - note: call /download?file="filename" to download a specific file
 * - `/status` to get the initialization status of ESP32 components
 * - `/battery` to get the battery data
 * - `/delete` to delete a file from the filesystem - note: call /delete?file="filename" to delete a specific file
 * 
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t http_server_start(void);

/**
 * Stop the HTTP server.
 * 
 * This function stops the HTTP server if it is running.
 * 
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t http_server_stop(void);

#endif // HTTP_SERVER_H