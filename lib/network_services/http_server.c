#include "http_server.h"

static const char *TAG = "HTTP_SERVER";


// Simple HTTP handler
static esp_err_t hello_get_handler(httpd_req_t *req) {
    const char* resp_str = "<html>\n"
                           "    <body><h1>Dog Collar GPS test for mDNS</h1>\n"
                           "    <p>mDNS test, I guess it is working :)</p></body>\n"
                           "</html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t list_files_get_handler(httpd_req_t *req) {

    const size_t RESPONSE_BUFFER_SIZE = 4096;
    char *response_buffer = (char *)malloc(RESPONSE_BUFFER_SIZE);

    if (response_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for file list response.");
        httpd_resp_send_500(req); // Send 500 Internal Server Error
        return ESP_FAIL; 
    }
    // Call your filesystem function to get HTML file list
    esp_err_t result = wifi_get_file_list_as_html(response_buffer, RESPONSE_BUFFER_SIZE);
    
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get file list from filesystem");
        free(response_buffer);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Set content type to HTML
    httpd_resp_set_type(req, "text/html");
    
    // Send the response
    httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);
    
    // Clean up
    free(response_buffer);
    return ESP_OK;
}



esp_err_t http_server_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_SERVER_PORT_NUM;
    
    httpd_handle_t server = NULL;

    // root URI handler
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t hello = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = hello_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &hello);

        // files URI handler
        httpd_uri_t files_uri = {
            .uri        = "/files",
            .method     = HTTP_GET,
            .handler    = list_files_get_handler, 
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &files_uri);

        ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
    return ESP_OK;
}

