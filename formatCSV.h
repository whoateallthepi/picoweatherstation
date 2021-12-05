#ifndef FORMATCSV_H_
#define FORMATCSV_H_

#define DELIM ";"

int csv_format_header (int type, char * hardwareID, datetime_t time,
						char  * timezone, char * buffer, int max_size);
					
int csv_format100 (char * hardwareID, datetime_t time,
                    char * timezone,
                    char * buffer, uint max_size, int32_t airT, 
                    int32_t rH, int32_t airP, float mslp, float windDir,
                    float windSp, float maxGust, 
					float maxGustD, float wind2mDir, float wind2mSp, 
					float wind10mGust, float wind10mGustD,
					float rain_today, float rain_1h, float rain_since_last );	
                         
#endif				
					 
					
					
						 						
						
						
						
					
						
					
