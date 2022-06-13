#ifndef FORMATMESSAGES_H_
#define FORMATMESSAGES_H_

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "types.h"

messageHeaderOut format_message_header (void);

/*
 * station100Message format_station_message (messageHeader mh, 
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
					  * */
					  
					  
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

stationReport format_station_report ();

int64_t station_report_callback(alarm_id_t id, void *user_data);
					  
void format_int16 (char * buffer, int16_t input);
void format_int32 (char * buffer, int32_t input);
	
                         
#endif				
					 
					
					
						 						
						
						
						
					
						
					
