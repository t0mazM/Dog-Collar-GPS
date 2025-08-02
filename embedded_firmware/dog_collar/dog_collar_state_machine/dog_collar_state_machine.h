#ifndef DOG_COLLAR_STATE_MACHINE_H
#define DOG_COLLAR_STATE_MACHINE_H

typedef enum {
    DOG_COLLAR_STATE_INITIALIZING, 
    DOG_COLLAR_STATE_NORMAL,

    DOG_COLLAR_STATE_LOW_BATTERY,
    DOG_COLLAR_STATE_CRITICAL_LOW_BATTERY,
    DOG_COLLAR_STATE_CHARGING,

    DOG_COLLAR_STATE_GPS_ACQUIRING,     
    DOG_COLLAR_STATE_GPS_READY,  
    DOG_COLLAR_STATE_GPS_FILE_CREATION,
    DOG_COLLAR_STATE_WAITING_FOR_GPS_FIX,  // We need to wait for fix to receive the data from gps to create a file and then start recording
    DOG_COLLAR_STATE_GPS_TRACKING,      
    DOG_COLLAR_STATE_GPS_PAUSED,  
    
    DOG_COLLAR_STATE_WIFI_SYNC, 

    DOG_COLLAR_STATE_LIGHT_SLEEP,
    DOG_COLLAR_STATE_DEEP_SLEEP,

    DOG_COLLAR_STATE_ERROR
} dog_collar_state_t;


#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "components_init/components_init.h"
#include "../components/battery_monitor/battery_monitor.h"
#include "../components/button_interupt/button_interrupt.h"
#include "../components/file_system_littlefs/file_system_littlefs.h"
#include "led_management/led_management.h" // Have to include this here to avoid circular dependency

#define BATTERY_SOC_HIGH 60.0f      // Battery is considered high if SOC is above this value
#define BATTERY_SOC_LOW 20.0f       // Battery is considered low if SOC is below this value
#define BATTERY_SOC_CRITICAL 10.0f  // Battery is considered critically low if SOC is below this value

#define BATTERY_CHECK_INTERVAL_MS_HIGH       5000 //300000 // 5 minutes -for testing 5s
#define BATTERY_CHECK_INTERVAL_MS_LOW        5000 //60000  // 1 minute -for testing 5s

#define WIFI_SYNC_TIME_S 60             //Time for one sync in seconds
#define WIFI_SYNC_PERIODIC_TIME_S 10    // Time between each Wi-Fi syncs in seconds 

#define LIGHT_SLEEP_MAX_COUNT 10        // After LIGHT_SLEEP_MAX_COUNT light sleeps, we will go for longer deep sleep.

#define LIGHT_SLEEP_TIME_S 60
#define DEEP_SLEEP_TIME_S 60 * 10 // 10 minutes

#define WAIT_AFTER_USER_PRESS_MS 500    // Wait after user press in milliseconds


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
dog_collar_state_t handle_initializing_state(void);


/**
 * @brief Handles the normal state of the dog collar.
 *
 * This function handles logic when the dog collar is idle:
 * - Waiting for user input to start GPS tracking.
 * - Periodically syncing with the server.
 *
 * @return dog_collar_state_t Wifi sync state if time to sync, otherwise normal state.
 */
dog_collar_state_t handle_normal_state(void);


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
dog_collar_state_t handle_low_battery_state(void);

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
dog_collar_state_t handle_critical_low_battery_state(void);

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
dog_collar_state_t handle_charging_state(void);

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
dog_collar_state_t handle_gps_acquiring_state(void);

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
dog_collar_state_t handle_gps_ready_state(void);

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
dog_collar_state_t handle_gps_file_creation_state(void);

/**
 * @brief Handles the waiting for GPS fix state of the dog collar.
 *
 * This state is entered when we pressed the button from the GPS acquiring state.
 * * It waits for the GPS to acquire a fix before starting the tracking session.
 * When the fix is acquired, it transitions to the GPS ready state.
 *
 * @return dog_collar_state_t gps_acquiring state if GPS fix is not acquired, gps_ready state if GPS is ready.
 */
dog_collar_state_t handle_waiting_for_gps_fix_state(void);

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
dog_collar_state_t handle_gps_tracking_state(void);

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
dog_collar_state_t handle_gps_paused_state(void);

/**
 * @brief Handles the Wi-Fi sync state of the dog collar.
 *
 * This state is entered periodically to sync data with the server.
 * - Connects to Wi-Fi.
 * - Tries to reach the server to sync data.
 *
 * @return dog_collar_state_t normal_state 
 */
dog_collar_state_t handle_wifi_sync_state(void);

/**
 * @brief Handles the light sleep state of the dog collar.
 *
 * - Turns off all LEDs.
 * - Puts the GPS L96 module into back-up mode.
 * - Puts the device into light sleep to save power.
 * - Wakes up on button press or after a certain period defined by LIGHT_SLEEP_TIME_S.
 * 
 *  After light sleep it will continue the state machine from the current state.
 *  It will light sleep for MAX_LIGHT_SLEEP_MAX_COUNT times in a row, then it will go to deep sleep.
 *  If it was woken up by a GPIO interrupt, it will enter the GPS_ACQUIRING state.
 *
 * @note Both light and deep sleep permanently turn off the USB UART, so we cannot use it to wake up the device.
 * @return dog_collar_state_t deep_sleep state if we have been in light sleep for too long, normal state if button is pressed.
 */
dog_collar_state_t  handle_light_sleep_state(void);

/**
 * @brief Handles the deep sleep state of the dog collar.
 *
 * - Turns off all LEDs.
 * - Puts the GPS L96 module into back-up mode.
 * - Puts the device into deep sleep to save power.
 * - Wakes up on timer or GPIO interrupt.
 *
 * After deep sleep the device will reset and start from the beginning of the program.
 * If it was woken up by a GPIO interrupt, it will enter the GPS_ACQUIRING state.
 *
 * @note The device will reset when waking up from deep sleep, so we will not return to this state.
 * @return dog_collar_state_t normal state, although we will not reach this point as we will reset the device.
 */
dog_collar_state_t  handle_deep_sleep_state(void);

/**
 * @brief Handles the error state of the dog collar.
 * This state is entered when any component fails to initialize or operate correctly.
 * - Logs the error and notifies the user with error LED animation.
 * - Stops all operations and enters a safe state.
 */
dog_collar_state_t handle_error_state(void);

/**
 * @brief Check if the device was woken up from sleep and return the GPS_ACQUIRING state if it was.
 *
 *
 * @param input_state The current state.
 * @return GPS_ACQUIRING if woken up by GPIO, otherwise returns the input state.
 */
dog_collar_state_t get_initial_state_from_wakeup(dog_collar_state_t input_state);



#endif // DOG_COLLAR_STATE_MACHINE_H