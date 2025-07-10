#include "http_server.h"

static const char *TAG = "HTTP_SERVER";

// Handlers prototypes
static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t list_files_get_handler(httpd_req_t *req);
static esp_err_t download_file_get_handler(httpd_req_t *req);

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

            // download files URI handler
            httpd_uri_t download_uri = {
            .uri        = "/download", // you get like http://dogcollar.local/download?file=my_log.txt
            .method     = HTTP_GET,
            .handler    = download_file_get_handler,
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &download_uri);

        ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
        return ESP_OK;
    } 

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return ESP_FAIL;
}

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
    char *response_buffer = (char *)malloc(RESPONSE_BUFFER_SIZE);

    if (response_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for file list response.");
        httpd_resp_send_500(req);
        return ESP_FAIL; 
    }

    // Get file list from filesystem in html format
    esp_err_t result = wifi_get_file_list_as_html(response_buffer, RESPONSE_BUFFER_SIZE);
    
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get file list from filesystem");
        free(response_buffer); 
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Set content type to HTML - check error but don't return yet
    esp_err_t err = httpd_resp_set_type(req, "text/html");
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set response type");
        free(response_buffer); 
        httpd_resp_send_500(req);
        return err;
    }

    // Send the response - check error but don't return yet
    err = httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send response");
        free(response_buffer); 
        return err;
    }
    ESP_LOGI(TAG, "File list sent successfully");
    free(response_buffer); 
    return ESP_OK;
}

static esp_err_t download_file_get_handler(httpd_req_t *req) {
    char filepath[256]; 
    char filename[128]; 

    // 1. Get the filename from the query parameter "file"
    // URI will look like /download?file=your_filename.csv
    if (httpd_query_key_value(req->uri, "file", filename, sizeof(filename)) != ESP_OK) {
        ESP_LOGE(TAG, "File parameter 'file' not found in URI query: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'file' parameter");
        //return ESP_FAIL;
    }

    // 2. Construct the full path on the file system
    //snprintf(filepath, sizeof(filepath), "/littlefs/%s", filename);
    ESP_LOGI(TAG, "Attempting to download file: %s", filepath);

  return ESP_OK;
}