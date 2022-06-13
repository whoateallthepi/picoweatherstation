#include <stdio.h>
#include <stdlib.h>
#include "hardware/rtc.h"
#include "formatmessages.h"
#include "utilities.h"
#include "constants.h"
#include "types.h"
#include <string.h> // for memcpy


//#define TRACE
//#define DEBUG

extern stationData stationdata;

messageHeaderOut format_message_header (void) {
    
    messageHeaderOut mh;
    
    datetime_t current_time;
    rtc_get_datetime(&current_time);
    
    time_t ts = rtc_to_epoch(current_time, stationdata.timezone);
    
    itoa(ts,mh.timestamp,16);
    
    format_int16(mh.timezone, stationdata.timezone);
    
    //itoa(stationdata.timezone, mh.timezone, 16);
    
    return mh;
}
 
/*messageHeader format_message_header (char messagetype) {
    messageHeader mh;
    
    mh.messagetype = messagetype;
    mh.boardID = stationdata.boardID;
    
    datetime_t current_time;
    rtc_get_datetime(&current_time);
    
    mh.timestamp = rtc_to_epoch(current_time, stationdata.timezone);
    
    return mh;
}
*/

/*
station100Message format_station_message (messageHeader mh, 
					  float temperature,
					  float humidity,
					  float pressure, 
					  float mslp, 
					  wind current_wind,
					  wind max_gust, 
					  wind wind2maverage,
					  wind wind10mmax,
					  float raintoday,
					  float rain1h,
					  float rainsincelast) {
    // all number parameters are floats now to force consistent 
    //decimal place handling. All reports are now in ints to compress 
    //the message. Decimal point logic is decoded in the client
    
    station100Message sm;
    sm.header = format_message_header(100);
    
    sm.temperature = float_to_int((temperature/100), DEFAULT_DECIMAL_PLACES);
    // For some reason temp is in hundredth of a degree
    sm.humidity = float_to_int((humidity/1000), DEFAULT_DECIMAL_PLACES);
    //humidity is not %, for some reason
    sm.bar_uncorrected = float_to_int32((pressure/100), DEFAULT_DECIMAL_PLACES);
    // output in hPa
    sm.bar_corrected = float_to_int32(mslp, DEFAULT_DECIMAL_PLACES);
    sm.wind_speed = float_to_int(current_wind.speed, DEFAULT_DECIMAL_PLACES);
    sm.wind_dir = radians_to_degrees(current_wind.direction);
    sm.wind_gust = float_to_int(max_gust.speed, DEFAULT_DECIMAL_PLACES);
    sm.wind_gust_dir = radians_to_degrees(max_gust.direction);
    sm.wind_speed_avg2m = float_to_int(wind2maverage.speed, DEFAULT_DECIMAL_PLACES);
    sm.wind_dir_avg2m = radians_to_degrees(wind2maverage.direction);
    sm.wind_gust_10m = float_to_int(wind10mmax.speed, DEFAULT_DECIMAL_PLACES);
    sm.wind_gust_dir_10m = radians_to_degrees(wind10mmax.speed);
    sm.rain_today = float_to_int(raintoday, DEFAULT_DECIMAL_PLACES);
    sm.rain_1h = float_to_int(rain1h, DEFAULT_DECIMAL_PLACES);
    sm.rain_since_last = float_to_int(rainsincelast, DEFAULT_DECIMAL_PLACES);
    
    return sm;
}
*/

weatherReport format_weather_report ( 
					  float temperature,
					  float humidity,
					  float pressure, 
					  float mslp, 
					  wind current_wind,
					  wind max_gust, 
					  wind wind2maverage,
					  wind wind10mmax,
					  float raintoday,
					  float rain1h,
					  float rainsincelast) {
    // all number parameters are floats now to force consistent 
    //decimal place handling. All reports are now in ints to compress 
    //the message. Decimal point logic is decoded in the client
    
    weatherReport wr;
    wr.header = format_message_header();
 
    format_int16(wr.temperature, float_to_int((temperature/100), DEFAULT_DECIMAL_PLACES));
    // For some reason temp is in hundredth of a degree
    format_int16(wr.humidity, float_to_int((humidity/1000), DEFAULT_DECIMAL_PLACES));
    //humidity is not %, for some reason
    format_int32(wr.bar_uncorrected, float_to_int32((pressure/100), DEFAULT_DECIMAL_PLACES));
    // output in hPa
    format_int32(wr.bar_corrected, float_to_int32(mslp, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.wind_speed, float_to_int(current_wind.speed, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.wind_dir, radians_to_degrees(current_wind.direction));
    format_int16(wr.wind_gust, float_to_int(max_gust.speed, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.wind_gust_dir, radians_to_degrees(max_gust.direction));
    format_int16(wr.wind_speed_avg2m, float_to_int(wind2maverage.speed, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.wind_dir_avg2m, radians_to_degrees(wind2maverage.direction));
    format_int16(wr.wind_gust_10m, float_to_int(wind10mmax.speed, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.wind_gust_dir_10m, radians_to_degrees(wind10mmax.direction));
    format_int16(wr.rain_today, float_to_int(raintoday, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.rain_1h, float_to_int(rain1h, DEFAULT_DECIMAL_PLACES));
    format_int16(wr.rain_since_last, float_to_int(rainsincelast, DEFAULT_DECIMAL_PLACES));
    
    wr.eos = '\0'; // Set up end of string to make it easier to use string functions later
    
    return wr;
}

stationReport format_station_report () {
    stationReport sr;
    sr.header = format_message_header();
    format_int32(sr.latitude,float_to_int(stationdata.latitude, 5));
    format_int32(sr.longitude,float_to_int(stationdata.longitude, 5));
    format_int16(sr.altitude,float_to_int(stationdata.altitude, 0));
    sr.eos = '\0'; // Set up end of string to make it easier to use string functions later
    return sr;
}

void format_int16 (char * buffer, int16_t input) {
    /* Take an int or uint and format as 4 hexadecimal characters
     * Basically this is sprintf, without the trailing \0
    */ 
    
    char temp[5];
    sprintf(temp, "%04x",input);
    memcpy(buffer, temp, 4);
    return;
}

void format_int32 (char * buffer, int32_t input) {
    /* Take an int or uint and format as 8 hexadecimal characters
     * Basically this is sprintf, without the trailing \0
    */ 
    
    char temp[9];
    sprintf(temp, "%08x",input);
    memcpy(buffer, temp, 8);
    return;
}
