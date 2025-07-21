#ifndef DOG_COLLAR_STATE_MACHINE_H
#define DOG_COLLAR_STATE_MACHINE_H

#include "components_init/components_init.h"

typedef enum {
    DOG_COLLAR_STATE_INITIALIZING, 
    DOG_COLLAR_STATE_NORMAL,

    DOG_COLLAR_STATE_LOW_BATTERY,
    DOG_COLLAR_STATE_CHARGING,

    DOG_COLLAR_STATE_GPS_ACQUIRING,     
    DOG_COLLAR_STATE_GPS_READY,         
    DOG_COLLAR_STATE_GPS_TRACKING,      
    DOG_COLLAR_STATE_GPS_PAUSED,  
    
    DOG_COLLAR_STATE_WIFI_SYNC, 

    DOG_COLLAR_STATE_LIGHT_SLEEP,
    DOG_COLLAR_STATE_DEEP_SLEEP,

    DOG_COLLAR_STATE_ERROR
} dog_collar_state_t;


static dog_collar_state_t handle_initializing_state(void);

static dog_collar_state_t handle_normal_state(void);

static dog_collar_state_t handle_low_battery_state(void);

static dog_collar_state_t handle_charging_state(void);

static dog_collar_state_t handle_gps_acquiring_state(void);

static dog_collar_state_t handle_gps_ready_state(void);

static dog_collar_state_t handle_gps_tracking_state(void);

static dog_collar_state_t handle_gps_paused_state(void);

static dog_collar_state_t handle_wifi_sync_state(void);

static dog_collar_state_t handle_light_sleep_state(void);

static dog_collar_state_t handle_deep_sleep_state(void);

static dog_collar_state_t handle_error_state(void);




#endif // DOG_COLLAR_STATE_MACHINE_H