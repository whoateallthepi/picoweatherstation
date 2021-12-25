#ifndef UTILITIES_H_
#define UTILITIES_H_
#include <stdint.h>
#include <time.h> 

#include "hardware/rtc.h"

int float_to_int(float in, float decimal_places);
int32_t float_to_int32(float in, float decimal_places);
time_t rtc_to_epoch (datetime_t rtc, int8_t timezone);
datetime_t epoch_to_rtc (time_t epoch);
float radians_to_degrees (float radians);
#endif
