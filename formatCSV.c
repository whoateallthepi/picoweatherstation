#include <stdio.h>
#include "hardware/rtc.h"
#include "formatCSV.h"

//#define TRACE
//#define DEBUG

const char * csvHeader = "%u" DELIM "%s" DELIM "%s%s" DELIM;
      // That's type, hardware id time and time zone
	 		  
/* message type 100 is the standard repeating data log message
 * field order is from the legacy system:
 * wind_dir, windspeed, windgust, windgustdir, 
 * windspeed2mavg, winddir2mavg, windgust10m, windgust10mdirection, 
 * humidity, tempC, rain1h, rain_today, rain_since_last (report) 
 * pressure, mslp
 */
 
const char * csv100 = "%.1f" DELIM "%.1f" DELIM "%.1f" DELIM "%.1f" DELIM
	"%.1f" DELIM "%.1f" DELIM "%.1f" DELIM "%.1f" DELIM
	"%.1f" DELIM "%.1f" DELIM "%.2f" DELIM "%.2f" DELIM "%.2f" DELIM
	"%d" DELIM "%.2f" DELIM;
		  
		  
int csv_format_header (int type, char * hardwareID, datetime_t time,
						char * timezone, char * buffer, int max_size) {
	#ifdef TRACE
      printf("> csv_format_header\n");
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
              
	int length = sprintf(buffer, csvHeader, type, hardwareID, time_s, timezone);
	
    #ifdef DEBUG
      printf("... buffer: %s\n", buffer);
    #endif  
	
	#ifdef TRACE
      printf("< csv_format_header\n");
    #endif 
    
	return length;
}

int csv_format100 (char * hardwareID, datetime_t time,
					char * timezone, 
                    char * buffer, uint max_size, int32_t airT, 
                    int32_t rH, int32_t airP, float mslp, float windDir,
                    float windSp, float maxGust, 
					float maxGustD, float wind2mDir, float wind2mSp, 
					float wind10mGust, float wind10mGustD, 
					float rain_today, float rain_1h, float rain_since_last) {
    #ifdef TRACE
      printf("> csv_format100\n");
    #endif 
    int length;
    char * p = buffer; //pointer 
    
    int chars = csv_format_header (100, hardwareID, time, timezone, buffer, max_size);
    
    #ifdef DEBUG_EXTRA
      for (int x=0; x < 200; x++) {
		  printf("... buffer [%u]: %c %X\n", x, buffer[x], buffer[x]);
	  }
    #endif  
    
    
    length = sprintf((p+chars), csv100, windDir, windSp, maxGust, maxGustD, wind2mSp,
					wind2mDir, wind10mGust, wind10mGustD, 
					(rH / 1024.0), (airT / 100.0), rain_1h, 
					rain_today, rain_since_last, airP, mslp);
					 
    
    #ifdef TRACE
      printf("< csv_format100\n");
    #endif 
    return length;
}

