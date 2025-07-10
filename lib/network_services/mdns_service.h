#ifndef MDNS_SERVICE_H
#define MDNS_SERVICE_H

#include "esp_err.h"
#include "mdns.h"



#define MDNS_HOST_NAME "dogcollar" 


esp_err_t start_mdns_service(void);



#endif // MDNS_SERVICE_H