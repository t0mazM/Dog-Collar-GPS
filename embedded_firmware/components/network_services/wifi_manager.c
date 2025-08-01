/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#include "wifi_manager.h"
#include "wifi_credentials.h"

static const char *TAG = "WIFI_APP";
static bool wifi_initialized = false;
static bool default_event_loop_created = false;

static esp_netif_t* sta_netif = NULL;

char *ssid = WIFI_CRED_SSID; 
char *password = WIFI_CRED_PASS; 

// Event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;


esp_err_t wifi_init(void) {
    if (wifi_initialized) {
        ESP_LOGW(TAG, "Wi-Fi already initialized");
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(nvs_flash_init(), 
                        TAG, "Failed to initialize NVS"
    );

    ESP_RETURN_ON_ERROR(wifi_connect_and_start_services(),
                        TAG, "Failed to connect to Wi-Fi network"
    );

    wifi_initialized = true;
    return ESP_OK;
}

/**
 * @brief Event handler for Wi-Fi events.
 * Handles connection, disconnection, and IP acquisition.
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(TAG, "Retrying to connect to SSID:%s, attempt %d", ssid, s_retry_num);
                break;

            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        
        mdns_service_start(); 
        http_server_start();  
    }
}

esp_err_t wifi_connect_and_start_services(void) { //TODO HANDLE ERRORS - return error
    s_wifi_event_group = xEventGroupCreate();

    // Initialize TCP/IP stack
    ESP_RETURN_ON_ERROR(esp_netif_init(),
                        TAG,
                        "Failed to initialize TCP/IP stack");

    // Create default event loop (can be called only once in the whole application lifetime)
    if (!default_event_loop_created) {
        esp_err_t err = esp_event_loop_create_default();
        if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
            default_event_loop_created = true;
        } else {
            ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(err));
            return err;
        }
    }

    if (sta_netif == NULL) {
        sta_netif = esp_netif_create_default_wifi_sta();
        if (sta_netif == NULL) {
            ESP_LOGE(TAG, "Failed to create default Wi-Fi station network interface");
            return ESP_FAIL;
        }
    }

    // Initialize Wi-Fi driver
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&wifi_init_config),
                        TAG,
                        "Failed to initialize Wi-Fi driver");

    // Register event handlers
    ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL),
                        TAG,
                        "Failed to register Wi-Fi event handler");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL),
                        TAG,
                        "Failed to register IP event handler");

    // Set Wi-Fi configuration
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = 0, 
            .sae_h2e_identifier = "",
        },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA),
                        TAG,
                        "Failed to set Wi-Fi mode");

    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config),
                        TAG,
                        "Failed to set Wi-Fi configuration");

    ESP_RETURN_ON_ERROR(esp_wifi_start(),
                        TAG,
                        "Failed to start Wi-Fi");

    ESP_LOGI(TAG, "wi-fi initialization complete.");

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(WIFI_MAX_CONNECTION_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID:%s", ssid);
        return ESP_OK;
    }
    
    // Early return for failure
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID:%s", ssid);
        return ESP_FAIL;
    }

    ESP_LOGE(TAG, "Connection timeout for SSID:%s", ssid);
    return ESP_ERR_TIMEOUT;

}


esp_err_t wifi_stop_all_services(void) {
    
    bool any_errors = false;

    // 1. Unregister/stop wifi event handlers that are running in the background/tas
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler);
    
    // 1. Stop http server
    if(http_server_stop() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop HTTP server");
        any_errors = true;
    }

    mdns_free(); //No return values
    ESP_LOGI(TAG, "mDNS service stopped");

    // 3. Disconnect from network
    if (esp_wifi_disconnect() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disconnect from WiFi");
        any_errors = true;
    }
    
    // Wait for clean disconnection
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 4. Stop WiFi driver
    if (esp_wifi_stop() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop WiFi");
        any_errors = true;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 5. Deinitialize WiFi driver
    if (esp_wifi_deinit() != ESP_OK) {
        ESP_LOGE(TAG, "WiFi deinit failed");
        any_errors = true;
    }

    /* 6. Destroy/delete network interface */
    if (sta_netif != NULL) {
        esp_netif_destroy(sta_netif);
        sta_netif = NULL;
    }

    // 7. Clean up event group
    if (s_wifi_event_group != NULL) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    // 8. Reset retry counter for next wifi connection
    s_retry_num = 0;

    // 9. Display final status
    if (any_errors) {
        ESP_LOGW(TAG, "WiFi shutdown completed with some errors");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "WiFi services shutdown completed successfully");
    wifi_initialized = false; // Reset the initialization flag
    return ESP_OK;
}

esp_err_t wifi_stop_all_services_retry(uint16_t _max_retry_count) {

esp_err_t error;
uint16_t retry_count = 0;;

do{
  error = wifi_stop_all_services();
  retry_count++;
  if (error == ESP_OK) {
      return ESP_OK;
  }
  ESP_LOGI(TAG, "Retrying WiFi shutdown (%d/%d)", retry_count, _max_retry_count);
}while(error != ESP_OK && retry_count < _max_retry_count);

return error;
}

esp_err_t wifi_manager_reconnect(void) {

    if(wifi_manager_is_initialized_and_connected()){
        return ESP_OK;
    }
    // Stop all services and deinit WiFi if already initialized
    if (wifi_initialized) {
        wifi_stop_all_services();
    }

    // Re-init and connect
    esp_err_t ret = wifi_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WiFi reconnected successfully.");
    } else {
        ESP_LOGE(TAG, "WiFi reconnection failed!");
    }
    return ret;
}

bool wifi_manager_is_initialized_and_connected(void) {
    if (s_wifi_event_group == NULL || !default_event_loop_created || !wifi_initialized) {
        ESP_LOGI(TAG, "Wifi not initialized");
        return false;
    }
    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    if ((bits & WIFI_CONNECTED_BIT) != 0) {
        ESP_LOGI(TAG, "WiFi is already connected. No need to reconect again");
        return true;
    }
    ESP_LOGI(TAG, "WiFi is not connected");
    return false;
}