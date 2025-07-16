#ifndef NMEA_COMMANDS_H
#define NMEA_COMMANDS_H


// Enable GLONASS only
#define GNSS_MODE_GLONASS_ONLY "$PMTK353,0,1,0,0,0*2A\r\n"

// Enable GPS only
#define GNSS_MODE_GPS_ONLY     "$PMTK353,1,0,0,0,0*2A\r\n"

// Enable GPS + GLONASS
#define GNSS_MODE_GPS_GLONASS  "$PMTK353,1,1,0,0,0*2B\r\n"

// Enable GALILEO only
#define GNSS_MODE_GALILEO_ONLY "$PMTK353,0,0,1,0,0*2A\r\n"

// Enable GPS + GALILEO
#define GNSS_MODE_GPS_ONLY "$PMTK353,1,0,0,0,0*2A\r\n"

// Enable GPS + GLONASS + Galileo
#define GNSS_MODE_GPS_GLONASS_GALILEO "$PMTK353,1,1,1,0,0*2A\r\n"


// Set GPS module in standby  mode
#define GPS_STAND_BY_MODE "$PMTK161,0*28\r\n"

// Set GPS module in sleep mode (must be woken up by setting pin FORCE_ON to HIGH)
#define GPS_SLEEP_MODE "$PMTK225,4*2F\r\n"

#define GPS_DEEP_SLEEP_MODE "$PMTK225,4*2F\r\n"

#define ONLY_GNRMC "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"

#define GNSS_SET_UPDATE_RATE_1HZ "$PMTK220,1000*1F\r\n" // Set update rate to 1Hz

#define GNSS_ENABLE_EASY  "$PMTK869,1,0*34\r\n" // Enable Easy mode - it stores the last position so on nexct gps start it gets fix faster


#endif // NMEA_COMMANDS_H
    