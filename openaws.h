#ifndef OPENAWS_H_
#define OPENAWS_H_
#include "types.h"
// ==================== function prototypes ============================

void on_uart_rx();                               // Setups
  
void setup_arrays(void); 

void setup_station_data(void);
void set_real_time_clock(void);
void open_uart(void);
void launch_core1(void);

int get_sector (uint16_t adc);
wind calc_average_wind(volatile const wind readings[], uint entries);
float radians_to_degrees (float radians);

void midnight_reset (void);                      // Reporting
void report_weather(int32_t humidity, int32_t pressure, int32_t temperature);
float adjust_pressure (int32_t pressure, int32_t altitude, float temperature);

void minute_processing (void);

bool core0_led_timer_callback(struct repeating_timer *t);
int64_t core0_led_alarm_callback(alarm_id_t id, void *user_data);    

#endif
