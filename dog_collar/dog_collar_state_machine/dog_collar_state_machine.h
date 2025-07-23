#ifndef DOG_COLLAR_STATE_MACHINE_H
#define DOG_COLLAR_STATE_MACHINE_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "components_init/components_init.h"
#include "../components/battery_monitor/battery_monitor.h"
#include "../components/button_interupt/button_interrupt.h"

#define WIFI_SYNC_TIME_S 20
#define SLEEP_TIME_S 10 

typedef enum {
    DOG_COLLAR_STATE_INITIALIZING, 
    DOG_COLLAR_STATE_NORMAL,

    DOG_COLLAR_STATE_LOW_BATTERY,
    DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY,
    DOG_COLLAR_STATE_CHARGING,

    DOG_COLLAR_STATE_GPS_ACQUIRING,     
    DOG_COLLAR_STATE_GPS_READY,  
    DOG_COLLAR_STATE_GPS_FILE_CREATION,      
    DOG_COLLAR_STATE_GPS_TRACKING,      
    DOG_COLLAR_STATE_GPS_PAUSED,  
    
    DOG_COLLAR_STATE_WIFI_SYNC, 

    DOG_COLLAR_STATE_ERROR
} dog_collar_state_t;

dog_collar_state_t dog_collar_state_machine_run(void);

/**
 * @brief Manages the battery state and performs actions based on the battery status.
 *
 * This function is called periodically to check:
 * TODO add docs here after implementing and testing the battery management routine
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
dog_collar_state_t battery_management_routine(dog_collar_state_t current_state);

/**
 * @brief Handles the initializing state of the dog collar.
 *
 * This function initializes all components of the dog collar.
 * If initialization is successful, it transitions to the normal state; otherwise, it goes to the error state.
 *
 * @return dog_collar_state_t The next state of the dog collar.
 */
static dog_collar_state_t handle_initializing_state(void);


/**
 * @brief Handles the normal state of the dog collar.
 *
 * This function handles logic when the dog collar is idle:
 * - Waiting for user input to start GPS tracking.
 * - Periodically syncing with the server.
 *
 * @return dog_collar_state_t Wifi sync state if time to sync, otherwise normal state.
 */
static dog_collar_state_t handle_normal_state(void);


/**
 * @brief Handles the low battery state of the dog collar.
 *
 * This function handles logic when the battery is low:
 * - Stops GPS tracking, shuts down GPS module.
 * - Notifies the user about low battery with a red LED blinking.
 * - Enters light sleep state to conserve power.
 *
 * @return dog_collar_state_t Light sleep state.
 */
static dog_collar_state_t handle_low_battery_state(void);

/**
 * @brief Handles the critical low battery state of the dog collar.
 *
 * This function handles logic when the battery is critically low:
 * - Does not notify the user about critical low battery to save power.
 * - Stops GPS tracking, shuts down GPS module.
 * - Enters deep sleep for a long duration to conserve power.
 *
 * @note When waking up from deep sleep, we return at main function and start program from the beginning.
 * @return dog_collar_state_t Normal state, although we will not reach this point as we will reset the device.
 */
static dog_collar_state_t handle_critical_low_battery_state(void);

/**
 * @brief Handles the charging state of the dog collar.
 *
 * - Turns on the charging LED animation.
 * - Turns on GPS module to get a GPS fix from cold start.
 * - Checks battery is charged and transitions to normal state when fully charged.
 *
 * @note If discharging while charging, the battery monitor routine will check if it has enough charge to continue operating.
 * @return dog_collar_state_t Charging state if not fully charged, normal state if fully charged.
 */
static dog_collar_state_t handle_charging_state(void);

/**
 * @brief Handles the GPS acquiring state of the dog collar.
 *
 * We enter this state when we press the button once.
 * It's purpose is to get the GPS fix while we and the dog are getting ready to head to running :)
 * - Waits/checks for GPS to acquire a fix.
 * - Showing acquiring LED animation.
 * - When GPS is ready, it transitions to the GPS ready state.
 *
 * @return dog_collar_state_t gps_acquiring state if GPS fix is not acquired, gps_ready state if GPS is ready.
 */
static dog_collar_state_t handle_gps_acquiring_state(void);

/**
 * @brief Handles the GPS ready state of the dog collar.
 *
 * This state is entered when the GPS has acquired a fix but we are not yet tracking.
 * After a button is pressed again we start tracking (first creating the file) and enter the GPS tracking state.
 * - Checks if the button is pressed to start GPS tracking.
 * - Showing ready LED animation.
 *
 * @return dog_collar_state_t gps_file_creation state if button is pressed, gps_ready state if no button press.
 */
static dog_collar_state_t handle_gps_ready_state(void);

/**
 * @brief Handles the GPS file creation state of the dog collar.
 *
 * This state is entered when we press the button again in the GPS ready state.
 * - Creates a new file for GPS tracking data.
 * - Starts the GPS tracking session.
 * - Saves the gps file name as a global variable.
 *
 * @return dog_collar_state_t gps_tracking state after file creation.
 */
static dog_collar_state_t handle_gps_file_creation_state(void);

/**
 * @brief Handles the GPS tracking state of the dog collar.
 *
 * This state is entered when the button is pressed in the GPS ready state and after the file is created.
 * - Starts GPS tracking and saves data to the file created in the previous state.
 * - Shows tracking LED animation.
 * - Checks if the button is pressed to pause GPS tracking.
 * - Goes to sleep state TODO: if we even have time to sleep
 *
 * @return dog_collar_state_t gps_paused state if button is pressed, gps_tracking state if no button press.
 */
static dog_collar_state_t handle_gps_tracking_state(void);

/**
 * @brief Handles the GPS paused state of the dog collar.
 *
 * This state is entered when the button is pressed in the GPS tracking state.
 * - Pauses GPS tracking - just stops saving data to the file, let the GPS module run.
 * - Shows paused LED animation.
 * - Checks if the button is pressed to resume GPS tracking.
 * - If we hold the button for a longer period, we finish the tracking session and go to the normal state.
 *
 * @return dog_collar_state_t gps_tracking state if button is pressed, gps_paused state if no button press or gps_normal state if button is held for a longer period.
 */
static dog_collar_state_t handle_gps_paused_state(void);

/**
 * @brief Handles the Wi-Fi sync state of the dog collar.
 *
 * This state is entered periodically to sync data with the server.
 * - Connects to Wi-Fi.
 * - Tries to reach the server to sync data.
 *
 * @return dog_collar_state_t normal_state 
 */
static dog_collar_state_t handle_wifi_sync_state(void);

/**
 * @brief Handles the error state of the dog collar.
 * This state is entered when any component fails to initialize or operate correctly.
 * - Logs the error and notifies the user with error LED animation.
 * - Stops all operations and enters a safe state.
 */
static dog_collar_state_t handle_error_state(void);




#endif // DOG_COLLAR_STATE_MACHINE_H