#include "types.h"
#include "hardware/timer.h"
#include "bme280.h" 

void core1_process (void);
void setup_arrays();
void gpio_callback(uint gpio, uint32_t events);
void get_wind_readings (void);
float get_wind_direction();
int get_sector (uint16_t adc);
uint64_t get_time(void);
static inline uint64_t raw_time_64(void);
//bool repeating_timer_callback_1s(struct repeating_timer *t); //every second
void second_processing(void);

extern volatile float rainHour[60];  
extern volatile float rainToday;
extern volatile float rainSinceLast;

extern volatile wind windspeed[120]; // 2 minute record of wind speed /dir
extern volatile wind gust10m[10]; // Fastest gust in past 10 minutes 
extern volatile wind max_gust; // daily max
extern wind volatile current_wind; // defined in openaws

// These three are continuously updated (10s) by core1_processing
// Initilaised and declared in openaws.c

extern uint32_t temperature;
extern uint32_t pressure;
extern uint32_t humidity; 
