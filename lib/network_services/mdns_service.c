#include "mdns_service.h"

const char *TAG = "MDNS_SERVICE";

esp_err_t start_mdns_service(void) {

    // Initialize mDNS service
    ESP_RETURN_ON_ERROR(mdns_init(),
                        TAG,
                        "Failed to initialize mDNS service");
    
    // Set hostname
    ESP_RETURN_ON_ERROR(mdns_hostname_set(MDNS_HOST_NAME),
                        TAG,
                        "Failed to set mDNS hostname");
    
    // Set instance name
    ESP_RETURN_ON_ERROR(mdns_instance_name_set("Dog Collar GPS Device"),
                        TAG,
                        "Failed to set mDNS instance name");

    ESP_LOGI(TAG, "mDNS responder started - hostname: %s", MDNS_HOST_NAME);
    return ESP_OK;
}