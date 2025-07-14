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


esp_err_t mdns_service_start(void);



#endif // MDNS_SERVICE_H