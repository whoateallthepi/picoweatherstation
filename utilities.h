#ifndef UTILITIES_H_
#define UTILITIES_H_
#include <stdint.h>
#include <time.h> 

#include "hardware/rtc.h"

void software_reset();

int float_to_int(float in, float decimal_places);
int32_t float_to_int32(float in, float decimal_places);
time_t rtc_to_epoch (datetime_t rtc, int8_t timezone);
void epoch_to_rtc (time_t epoch, datetime_t * rtc);
float radians_to_degrees (float radians);

int getNum(char ch);
int32_t hex2int32 (char * hex);
int16_t hex2int16 (char * hex);
int hex2int(char * hex);

void setup_led(uint led);
void led_on(uint led);
void led_off(uint led);
void led_flash(uint led);  
void led_double_flash(uint led);
void led_repeat_flash(uint led);
void led_repeat_two_leds_alternate (uint led1, uint led2);
void led_flash2_leds (uint led1, uint led2);
void led_flash3_leds (uint led1, uint led2, uint led3);

int bytes_compare (const char * bytes1, const char * bytes2, const int bytes);

#endif
