#ifndef TYPES_H_
#define TYPES_H_

#include "constants.h"
#include "pico/unique_id.h" 

typedef struct {
    float speed;  
    float direction; // in Radians
} wind;

typedef struct { 
    char hardwareID[HARDWARE_ID_LENGTH]; // stored in hex string for usability
    pico_unique_board_id_t boardID; // to replace above
    char name[STATION_NAME_LENGTH];
    int8_t timezone; //  
    char position[22]; // Lat long eg 53.27044,-1.62351
    int32_t altitude; } stationData;
    
/*typedef struct {
    char messagetype;
    pico_unique_board_id_t boardID;
    time_t timestamp; 
    int16_t timezone; } messageHeader;
    
    
typedef struct {
    messageHeader header;
    uint16_t wind_dir;
    uint16_t wind_speed;
    uint16_t wind_gust;
    uint16_t wind_gust_dir;
    uint16_t wind_speed_avg2m;
    uint16_t wind_dir_avg2m;
    uint16_t wind_gust_10m;
    uint16_t wind_gust_dir_10m;
    uint16_t humidity;
    int16_t temperature;
    uint16_t rain_1h;
    uint16_t rain_today; 
    uint16_t rain_since_last;
    uint32_t bar_uncorrected;
    uint32_t bar_corrected; } station100Message;
    
    
*/    
    
typedef struct {
    char messagetype [2];
    char hardwareID [HARDWARE_ID_LENGTH - 1]; // no null needed
    char timestamp [8]; 
    char timezone [4]; } messageHeader; // now an epoch time
    
typedef struct {
    messageHeader header;
    char wind_dir [4];
    char wind_speed [4];
    char wind_gust [4];
    char wind_gust_dir [4];
    char wind_speed_avg2m [4];
    char wind_dir_avg2m [4];
    char wind_gust_10m [4];
    char wind_gust_dir_10m [4];
    char humidity [4];
    char temperature [4];
    char rain_1h [4];
    char rain_today [4]; 
    char rain_since_last [4];
    char bar_uncorrected [8];
    char bar_corrected [8]; 
    char eos; } weatherReport;    // end of string char
 
// ==================== incoming mesages =========================

typedef struct {
    char timestamp [8];
    char timezone [4]; } timeMessage200;
    
typedef struct {
    char not_implemented_yet [20]; } stationDataMessage201;    
    
typedef union {
    char message_ch [RX_BUFFER_SIZE]; // to be on the safe side
    stationDataMessage201 stationdatamessage;
    timeMessage200 timemessage; } incomingData;
    
typedef struct { 
    char messagetype [2];
    char hardwareID [HARDWARE_ID_LENGTH - 1]; // no null needed
    incomingData incomingdata;
    char eos; } incomingMessage;
        
   
#endif
