
#include "utilities.h"
#include <math.h>
#include <time.h>

#include "constants.h"

int float_to_int(float in, float decimal_places) {
/* Takes a floating point number and returns an int with the required number of
decimal places incorporated 
eg float_to_int(37.6666,2) returns 3767 
works with negative numbers and should work with negative decimal places - if you really 
want
*/
    float shifted;
    float factor = powf(10,decimal_places);
    
    if (in >= 0) {
        shifted = (int) (in * factor + .5);
    }
    else {
        shifted = (int) (in * factor - .5);
    }
    
    return shifted;
}


int32_t float_to_int32(float in, float decimal_places) {
/* Takes a floating point number and returns an int with the required number of
decimal places incorporated 
eg float_to_int(37.6666,2) returns 3767 
works with negative numbers and should work with negative decimal places - if you really 
want
*/
    float shifted;
    float factor = powf(10,decimal_places);
    
    if (in >= 0) {
        shifted = (int) (in * factor + .5);
    }
    else {
        shifted = (int) (in * factor - .5);
    }
    
    return shifted;
}


time_t rtc_to_epoch (datetime_t rtc, int8_t timezone) {
    struct tm t;
    t.tm_year = (rtc.year - 1900);
    t.tm_mon = (rtc.month -1) ;     //  0 = jan
    t.tm_mday = rtc.day;
    t.tm_hour = rtc.hour;
    t.tm_min = rtc.min;
    t.tm_sec = rtc.sec;
    t.tm_isdst = timezone;
    
    return mktime(&t);
}
  
datetime_t epoch_to_rtc (time_t epoch) {
    
    struct tm t;
    datetime_t rtc;
    
    t = *localtime(&epoch);
    
    rtc.year = (t.tm_year + 1900);
    rtc.month = (t.tm_mon + 1);     //  1 = jan
    rtc.day = t.tm_mday;
    rtc.hour = t.tm_hour;
    rtc.min = t.tm_min;
    rtc.sec = t.tm_sec;
    rtc.dotw = t.tm_wday;
}
float radians_to_degrees (float radians) {
     #ifdef TRACE
      printf("> radians_to_degrees\n");
    #endif
    
    return (radians * (180 / PI));
   
    #ifdef TRACE
      printf("< radians_to_degrees\n");
    #endif    
} 
