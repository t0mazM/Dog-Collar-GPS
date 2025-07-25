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



esp_err_t http_server_start(void);
esp_err_t http_server_stop(void);

#endif // HTTP_SERVER_H