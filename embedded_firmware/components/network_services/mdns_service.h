/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef MDNS_SERVICE_H
#define MDNS_SERVICE_H

#include "esp_err.h"
#include "mdns.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h" 
#include "esp_check.h"
#include "esp_log.h"


#define MDNS_HOST_NAME "dogcollar" 

/** Start the mDNS service.
 *
 * This function initializes and starts the mDNS service.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t mdns_service_start(void);



#endif // MDNS_SERVICE_H