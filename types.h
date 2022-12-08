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
    uint16_hex2int8(data->incomingdata.timemessage.timezone);t wind_gust;
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
    char timestamp [8]; // seconds from BASELINE_TIME
    char timezone [2]; } messageHeaderOut; 

typedef struct {
    messageHeaderOut header;
    char wind_dir [3]; // these are all numbers of hex chars eg
    char wind_speed [4];
    char wind_gust [4];
    char wind_gust_dir [3];
    char wind_speed_avg2m [4];
    char wind_dir_avg2m [3];
    char wind_gust_10m [4];
    char wind_gust_dir_10m [3];
    char humidity [4];
    char temperature [4];
    char rain_1h [4];
    char rain_today [4]; 
    char rain_since_last [4];
    char bar_uncorrected [4]; // Pressure from BASELINE_PRESSURE
    char bar_corrected [4]; 
    char battery_voltage [4]; // Only needs 3 char but modem needs whole chars
    char eos; } weatherReport;    // end of string char

typedef struct {
    messageHeaderOut header;
    char latitude [8];
    char longitude [8];
    char altitude [4];
    char eos; } stationReport;
 
// ==================== incoming mesages =========================

typedef struct {
    char timezone [1]; } timeMessage200;
    
typedef struct {
    char latitude [8];
    char longitude [8];
    char altitude [4]; } stationDataMessage201;    
    
typedef union {
    char message_ch [RX_BUFFER_SIZE]; // to be on the safe side
    stationDataMessage201 stationdatamessage;
    timeMessage200 timemessage; } incomingData;
    
typedef struct { 
    //char messagetype [2];
    incomingData incomingdata;
    char eos; } incomingMessage;
        
   
#endif
