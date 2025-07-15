#include "http_server.h"

static const char *TAG = "HTTP_SERVER";
httpd_handle_t server = NULL; //

// Handlers prototypes
static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t list_files_get_handler(httpd_req_t *req);
static esp_err_t download_file_get_handler(httpd_req_t *req);
static esp_err_t status_get_handler(httpd_req_t *req);

esp_err_t http_server_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_SERVER_PORT_NUM;
    
    server = NULL;

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

        /* Display status of ESP32 components and battery */
        httpd_uri_t status_uri = {
            .uri        = "/status",
            .method     = HTTP_GET,
            .handler    = status_get_handler, 
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &status_uri);

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

    char read_buffer[CHUNK_BUFFER_SIZE]; // TODO: Have DOWLOAD_BUFFER_SIZE be 4096, , and then send that in chunks off 1460
    lfs_ssize_t bytes_read = 0;
    uint16_t num_off_chunks = 1;
    lfs_file_t file;
 
    // Check if the request URI contains the 'file' parameter
    // URI will look like /download?file=your_filename.csv
    char *query_string = strchr(req->uri, '?');
    if (query_string == NULL) {
        ESP_LOGE(TAG, "No query string found in URI");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'file' parameter");
        return ESP_FAIL;
    }
    query_string++; // Move pointer past the '?' character
    ESP_LOGI(TAG, "Query string found: %s", query_string);


    query_string = query_string + 5; // Skip "file=" part

    ESP_LOGI(TAG, "Filename extracted: %s", query_string);

    // Read file (read only mode)
    esp_err_t err = lfs_file_open(&lfs, &file, query_string, LFS_O_RDONLY); //lfs is global extern variable from file_system_littlefs.c
    if (err) {
        ESP_LOGE(TAG, "Failed to open file %s for reading (%d)", query_string, err);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File not found or cannot be opened");
        return ESP_FAIL;
    }

    // Set content disposition to download the file directly
    char content_disposition[LFS_NAME_MAX + 64]; // Add extra space for disposition header
    snprintf(content_disposition, sizeof(content_disposition), "attachment; filename=\"%s\"", query_string);
    httpd_resp_set_hdr(req, "Content-Disposition", content_disposition);

    // Loop to read and send file in chunks
    do {
        bytes_read = lfs_file_read(&lfs, &file, read_buffer, sizeof(read_buffer));
        if (bytes_read < 0) {

            ESP_LOGE(TAG, "Failed to read from file %s (%d)", query_string, (int)bytes_read);
            lfs_file_close(&lfs, &file);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read file");
            return ESP_FAIL;

        } else if (bytes_read > 0) {
            // Send the chunk
            if (httpd_resp_send_chunk(req, read_buffer, bytes_read) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send file chunk");
                lfs_file_close(&lfs, &file);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file chunk");
                return ESP_FAIL; // Client disconnected or error
            }
            ESP_LOGI(TAG, "Sent chunk %d of %s (%d bytes)", num_off_chunks++, query_string, (int)bytes_read);
        }
    } while (bytes_read > 0);

    // After sending all data, send an empty chunk to signify end of transfer
    httpd_resp_send_chunk(req, NULL, 0); // This signals end of data for chunked transfer

    lfs_file_close(&lfs, &file);
    ESP_LOGI(TAG, "File %s sent successfully", query_string);
    return ESP_OK;
}

static esp_err_t status_get_handler(httpd_req_t *req) {

    char status_buffer[256];
    int status_length = dog_collar_get_status_string(status_buffer, sizeof(status_buffer));
    
    if (status_length < 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get status off ESP32 components and the battery");
        return ESP_FAIL;
    }

    ESP_RETURN_ON_ERROR(httpd_resp_set_type(req, "text/plain"),
                        TAG, "Failed to set response type to text/plain");

    ESP_RETURN_ON_ERROR(httpd_resp_send(req, status_buffer, status_length),
                        TAG, "Failed to send status response");
    return ESP_OK;
}

esp_err_t http_server_stop(void) {
    if (server != NULL) {
        ESP_LOGI(TAG, "Stopping HTTP server");
        esp_err_t err = httpd_stop(server);
        if (err == ESP_OK) {
            server = NULL;  // Clear the server handle
            ESP_LOGI(TAG, "HTTP server stopped successfully");
        } else {
            ESP_LOGE(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(err));
        }
        return err;
    } else {
        ESP_LOGW(TAG, "HTTP server is not running");
        return ESP_OK;  // Not an error if already stopped
    }
}