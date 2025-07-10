#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"

#include "file_system_littlefs.h"

#define RESPONSE_BUFFER_SIZE 4096
#define HTTP_SERVER_PORT_NUM 80

esp_err_t http_server_start(void);

// Handlers
static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t list_files_get_handler(httpd_req_t *req);

#endif // HTTP_SERVER_H