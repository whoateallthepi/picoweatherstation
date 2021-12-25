#ifndef FORMATMESSAGES_H_
#define FORMATMESSAGES_H_

#include "hardware/rtc.h"
#include "types.h"

messageHeader format_message_header (char messagetype);

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
					  float rainsincelast);
					  
					  
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
					  float rainsincelast);
					  
void format_int16 (char * buffer, int16_t input);
void format_int32 (char * buffer, int32_t input);
	
                         
#endif				
					 
					
					
						 						
						
						
						
					
						
					
