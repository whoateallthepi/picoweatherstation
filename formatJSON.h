int json_format_header (int type, char * hardwareID, datetime_t time,
						char  * timezone, char * buffer, int max_size);
					
int json_format100 (char * hardwareID, datetime_t time,
                    char * timezone,
                    char * buffer, uint max_size, int32_t airT, 
                    int32_t rH, int32_t airP, int32_t mslp, uint windDir,
                    float windSp, float maxGust, 
					uint maxGustD, uint wind2mDir, float wind2mSp, 
					float wind10mGust, uint wind10mGustD,
					float rain_today, float rain_1h, float rain_since_last );					
					 
					
					
						 						
						
						
						
					
						
					
