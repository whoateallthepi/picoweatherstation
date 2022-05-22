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
    int8_t timezone; //  
    float latitude; 
    float longitude; // Lat long eg 53.27044,-1.62351
    int32_t altitude; 
    int8_t network_status; 
    int8_t send_station_report;} stationData;
    
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
    char timestamp [8]; // epoch time
    char timezone [4]; } messageHeaderOut; 

typedef struct {
    messageHeaderOut header;
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

typedef struct {
    messageHeaderOut header;
    char latitude [8];
    char longitude [8];
    char altitude [4];
    char eos; } stationReport;
 
// ==================== incoming mesages =========================

typedef struct {
    char timezone [4]; } timeMessage200;
    
typedef struct {
    char latitude [8];
    char longitude [8];
    char altitude [4]; } stationDataMessage201;    
    
typedef union {
    char message_ch [RX_BUFFER_SIZE]; // to be on the safe side
    stationDataMessage201 stationdatamessage;
    timeMessage200 timemessage; } incomingData;
    
typedef struct { 
    char messagetype [2];
    incomingData incomingdata;
    char eos; } incomingMessage;
        
   
#endif
