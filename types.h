#ifndef TYPES_H_
#define TYPES_H_

#include "constants.h"

typedef struct {
    float speed;  
    float direction; // in Radians
} wind;

typedef struct { 
    char hardwareID[HARDWARE_ID_LENGTH]; // stored in hex string for usability
    char name[STATION_NAME_LENGTH];
    char timezone[7]; // Using ISO 8601 notation ie Z or +01:00 etc
    char position[22]; // Lat long eg 53.27044,-1.62351
    int32_t altitude; } stationData;

#endif
