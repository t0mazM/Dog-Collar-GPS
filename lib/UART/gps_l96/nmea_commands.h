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

// Set update rate for GPS to 1Hz
#define GNSS_SET_UPDATE_RATE_1HZ "$PMTK220,1000*1F\r\n"


#endif // NMEA_COMMANDS_H
    