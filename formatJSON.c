#include <stdio.h>
#include "hardware/rtc.h"
#include "formatJSON.h"

//#define TRACE
//#define DEBUG

const char * jsonHeader = 
	"{ \"AWSMessage\": "
		"{ \"Type\" : %u, "
		  "\"HardwareID\" : \"%s\","
		  "\"Time\" : \"%s%s\","; // Message types need an extra } to close
		  
// message type 100 is the standard repeating data log message
const char * json100 = 		  		  
		  " \"Data\" : { "
				"\"PTU\" : { "
					"\"Air-Temperature\" : \"%.2fC\","
					"\"Relative-Humidity\" : \"%.2f%%\","
					"\"Air-Pressure\" : \"%dPa\","
					"\"mslp\" : \"%dPa\""
				"},"
				" \"WIND\" : { "
					"\"LATEST\" : { "
						"\"Direction\" : %u,"
						"\"Speed\" : \"%.1f\" ,"
						"\"Daily-Max-Gust\" : \"%.1f\","
						"\"Daily-Max-Gust-Direction\" : %u },"
					" \"2MINUTES\" : { "
						"\"Direction\" : %u,"
						"\"Speed\" : \"%.1f\" },"
					" \"10MINUTES\" : { "
						"\"Max-Gust\" : \"%.1f\","
						"\"Max-Gust-Direction\" : %u } }," 
				" \"PRECIPITATION\" : { "
						"\" Rain\" : { \"today\" : \"%.2fmm\","
										"\"1hour\" : \"%.2fmm\", "
										"\"since-last\" : \"%.2fmm\" }}}}\n";
						

int json_format_header (int type, char * hardwareID, datetime_t time,
						char * timezone, char * buffer, int max_size) {
	#ifdef TRACE
      printf("> json_format_header\n");
    #endif 
    
    // stringify the time
    char time_s[20];
    sprintf(time_s,"%i-%i-%iT%02i:%02i:%02i", 
			time.year,
            time.month,
            time.day,
            time.hour,
            time.min,
            time.sec);
              
	int length = sprintf(buffer, jsonHeader, type, hardwareID, time_s, timezone);
	
	#ifdef DEBUG
      printf("... buffer: %s\n", buffer);
    #endif  
	
	#ifdef TRACE
      printf("< json_format_header\n");
    #endif 
    
	return length;
}

int json_format100 (char * hardwareID, datetime_t time,
					char * timezone, 
                    char * buffer, uint max_size, int32_t airT, 
                    int32_t rH, int32_t airP, int32_t mslp, uint windDir,
                    float windSp, float maxGust, 
					uint maxGustD, uint wind2mDir, float wind2mSp, 
					float wind10mGust, uint wind10mGustD, 
					float rain_today, float rain_1h, float rain_since_last) {
	#ifdef TRACE
      printf("> json_format100\n");
    #endif 
    int length;
    char * p = buffer; //pointer 
    
    int chars = json_format_header (100, hardwareID, time, timezone, buffer, max_size);
    
    #ifdef DEBUG_EXTRA
      for (int x=0; x < 200; x++) {
		  printf("... buffer [%u]: %c %X\n", x, buffer[x], buffer[x]);
	  }
    #endif  
    
    
    length = sprintf((p+chars), json100,
					 (airT / 100.0), (rH / 1024.0), airP, mslp, windDir, windSp, maxGust,
					 maxGustD, wind2mDir, wind2mSp, wind10mGust,
					 wind10mGustD, rain_today, rain_1h, rain_since_last);
    
    #ifdef TRACE
      printf("< json_format100\n");
    #endif 
    return length;
}

