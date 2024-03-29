/* ================ Welcome to multicore! =============================
 * 
 * This version splits the processing. Core 0 handles the interface with
 * the modem (RAK811), plus all the reporting, timing, clock etc.
 * Core 1 handles the readings of the rain/wind interrupts, the ADC (for
 * wind direction)
 * Core 0 also read the temperature/pressure/humidity readings
 * (BMP280) as part of the reporting function.  
 *
 * Communication between the cores is via a series of file scope variables.
 * A more formal FIFO mechanism for intercore communication is available
 * - but this would be (?) overkill in this case. 
 * 
 * All the sensor/interupt values are written in core 1. This is done in 
 * a loop taking readings every second or so.
 * 
 * Core zero does the reporting each minute (or as set by a station
 * vartiable when I get around to writing the code).
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/unique_id.h"
#include "pico/multicore.h"
#include "hardware/timer.h"

#include "constants.h"
#include "types.h"
#include "openaws.h"

#include "formatmessages.h" // new code for handling binary data transfers
                            // will supercede formatCSV
//#include "uart_lora.h"      // Code for IO / initialisation of the UART
//#include "rak811.h"         // The LORA modem unit
#include "rak811_lorawan.h" // New code to use lowrawan rather than P2P - will supercede rak811.h

#include "core1_processing.h"
#include "bme280.h"
#include "utilities.h"
#include "ds3231.h"

#define DS3231 // include the code for external clock

//#define DEBUG
//#define TRACE
//#undef  DEBUG
//#undef TRACE

/* Wiring for BME280 - #define(d) in bme280.h
   All now processed via core1_processing.c

   GPIO 16 (pin 21) MISO/spi0_rx-> SDO/SDO on bme280 board
   GPIO 17 (pin 22) Chip select -> CSB/!CS on bme280 board
   GPIO 18 (pin 24) SCK/spi0_sclk -> SCL/SCK on bme280 board
   GPIO 19 (pin 25) MOSI/spi0_tx -> SDA/SDI on bme280 board
   3.3v (pin 3;6) -> VCC on bme280 board
   GND (pin 38)  -> GND on bme280 board
*/

/* ========================= tracking these readings ===================

Wind speed and direction each time through the loop (not stored)
Wind gust mx and direction on over the day (not stored)
Wind speed and direction over 2 minutes (stored once per second)
Wind gust/  direction over 10 minutes - store one per minute
Rain over the past hour
Total rain over the day 
*/

// ====================File scope variables ============================

stationData stationdata;

// These three are continuously updated (10s) by core1_processing
int32_t temperature;
uint32_t pressure;
uint32_t humidity;
float decimal_temperature;

int sendstationreport = 0; // set to force main loop to ping out a station status (message 101)

// ==================== volatiles - set in IRQs by core_1 ==============

volatile float rainHour[60];
volatile float rainToday = 0;
volatile float rainSinceLast = 0;

// == counters and totals - maintained by core1_processing =============

volatile wind windspeed[120]; // 2 minute record of wind speed /direach second

volatile wind gust10m[10]; // Fastest gust in past 10 minutes

volatile wind max_gust = {.speed = 0, .direction = 0}; // daily max

volatile wind current_wind = {.speed = 0, .direction = 0}; // latest

volatile wind current_wind_5s = {.speed = 0, .direction = 0}; // latest 5s average

volatile uint16_t battery_level; // updated via ADC in core1

// =========================== main ====================================

int main(void)
{

  uint64_t now;
  uint64_t core0_led_on_time = 0;
  uint64_t core0_led_off_time = 0;
  uint core0_led_state = 0;       // off
  uint32_t time_message_received; // Used with time sync message

  char seconds = 0;     // Controls the main loop
  char seconds_2m = 0;  // index for the 2m readings wind speed/dir
  char minutes_10m = 0; // index of wind gust dir/speed over 10m

  datetime_t current_time;

  uint8_t RX_buffer_copy[RX_BUFFER_SIZE]; //working copy of the buffer

  struct repeating_timer led0_timer;

  // ====================== Zero-fill arrays Start RTC ===================

#if defined TRACE || defined DEBUG
  stdio_init_all();
#endif

#if defined TRACE
  sleep_ms(10000); // give time to start serial terminal
  printf("> main()\n");
#endif

  setup_led(CORE0_LED);
  setup_led(TX_LED);
  setup_led(RX_LED);

  // Briefly flash all leds to show we are starting OK
  led_on(CORE0_LED);
  led_on(TX_LED);
  led_on(RX_LED);
  busy_wait_us_32(BOOT_LED_FLASH);
  led_off(CORE0_LED);
  led_off(TX_LED);
  led_off(RX_LED);

  //add_repeating_timer_ms(-CORE0_LED_INTERVAL, core0_led_timer_callback, NULL, &led0_timer);

  set_real_time_clock();
  setup_arrays();

  setup_station_data(); // HardwareID

  stationdata.network_status = NETWORK_INITIALISING;

  if (rak811_lorawan_initialise())
  {
    stationdata.network_status = NETWORK_MODEM_ERROR;
    led_double_flash(RX_LED);
  }
  else
  {
    rak811_lorawan_join();
  }
   
   // sleep the modem to save power. Not checking error as there is litte we can do
   
   rak811_change_state(SLEEP);

  // Pressure, temp and humidity sensor

  if (bme280_initialise())
  {
    printf("Error: failed to initialise BME280\n");
    led_repeat_flash(CORE0_LED);
    //need to flash LEDs
  }

#ifdef DEBUG
  printf("...about to launch core1\n");
#endif

  multicore_launch_core1(core1_process); // handles all the sensors
                                         // consider a delay/ack here

  /* ============= set up the 0 seconds alarm ================ =======
* When the RTC hits 0s the various reporting states are checked
*/

  datetime_t minute_alarm = {
      .year = -1,
      .month = -1,
      .day = -1,
      .dotw = -1,
      .hour = -1,
      .min = -1,
      .sec = 0};

  rtc_enable_alarm();
  rtc_set_alarm(&minute_alarm, &minute_processing);

  // =========================== tight loop  ==============================

  while (1)
  {

    now = time_us_32();

    if (core0_led_state)
    {
      // LED is ON - has it been lit for more than LED_FLASH
      if ((now - core0_led_on_time) > LED_FLASH)
      {
        led_off(CORE0_LED);
        core0_led_off_time = now;
        core0_led_state = 0;
      }
    }
    else
    {
      // LED is OFF has it been out for more than LED INTERVAL
      if ((now - core0_led_off_time) > CORE0_LED_INTERVAL)
      {
        led_on(CORE0_LED);
        core0_led_on_time = now;
        core0_led_state = 1;
      }
    }

  } // tight loop
} // main

// ====================== supporting functions =========================

// ========================== report weather routines ==================

void minute_processing(void)
{
#ifdef TRACE
  printf("> minute processing\n");
#endif
  datetime_t current_time;

  rtc_get_datetime(&current_time);

  if ((current_time.min == 0) || (current_time.min % REPORTING_FREQUENCY) == 0)
  {

    bme280_fetch(&humidity, &pressure, &temperature);

    report_weather(humidity, pressure, temperature);

    // =============midnight processing=======================

    if (current_time.min == 0)
    {
      //check if it's midnight, local time. The clock is always UTC)
      int8_t adjusted_hour = (current_time.hour + stationdata.timezone);
      if (adjusted_hour == 0 || adjusted_hour == 24)
      {
        midnight_reset();
        stationdata.send_station_report = 1; // ping off a station status report soon after midnight
      }
    }
  }
  else
  {
    /*This is executed every minute, except the minutes when a weather report is sent
    *
    * If the sendstationreport flag is set (either above or because either message 200 or 201 
    * has been received) send it. This shouldn't clash with weather report processing, 
    * but if REPORTING_FREQUENCY = 1, it will never execute
    */
    if (stationdata.send_station_report)
    {
      //add_alarm_in_ms(25000, station_report_callback, NULL, false);
      stationdata.send_station_report = 0;
      //busy_wait_ms(STATION_REPORT_DELAY);
      report_station();
    }
  }
}

void report_station (void) 
{
  stationReport sr;

  sr = format_station_report();

  // next station message to rak11 format

  rak811_change_state(WAKE);

  rak811_lorawan_put_hex((char *)&sr, sizeof(stationReport), 101); // treat the structure as bytes
  
  rak811_change_state(SLEEP);

}

void report_weather(int32_t humidity, int32_t pressure, int32_t temperature)
{
#ifdef TRACE
  printf("> report_weather\n");
#endif
  int type = 100;
  char hardware[] = "hardware id";
  char tt[] = "The time";
  char buffer[1000];
  datetime_t report_time;
  float rain_1h = 0;
  float mslp = 0;
  float battery_voltage;

  // Add up the rainHour array for the hourly rain total

  for (int x = 0; x < 60; x++)
    rain_1h += rainHour[x];

  //  Wind

  // 10m gusts

  wind gust10max = {0, 0};

  for (int x = 0; x < 10; x++)
  {
    if (gust10m[x].speed > gust10max.speed)
    {
      gust10max.speed = gust10m[x].speed;
      gust10max.direction = gust10m[x].direction;
    }
  }

#ifdef DEBUG
  printf("...10m gust: %.2f\n", gust10max.speed);
  printf("...10m dir: %.2f\n", gust10max.direction);
#endif

  // ====================== unit testing =============================
  //#ifdef DEBUG  // unit test data
  /* For some reason removing this first call to calc_average_wind
     * causes a crash. I still can't work out why!
     * 
    */
  wind testdata[3] = {
      {25.0, 6.6},
      {18, 0.78},
      {18, .39}};

  wind averages = {0, 0};
  //printf("x\n");
  averages = calc_average_wind(testdata, 3);

  //printf("...average speed = %.2f\n", averages.speed);
  //printf("...average direction = %.2f\n", averages.direction);
  //#endif
  // ================== end unit testing =============================

  // 2m averages

  wind averages2m = {0, 0};

  averages2m = calc_average_wind(windspeed, 120);

  /* sea level pressure - this works for elevations up to a 
     * few hundred metres
     * Our readings are in Pascals
     * 
     * Each metre of elevation reduces pressure by 12 Pa
     * See https://www.rmets.org/metmatters/feeling-under-pressure-what-are-barometers)
     * 
     * mslp =  station pressure + (elevation * 12)  
     * 
    */

  mslp = adjust_pressure(pressure, stationdata.altitude, decimal_temperature);

#ifdef DEBUG
  printf("Humidity = %.2f%%\n", humidity / 1024.0);
  printf("Pressure = %dPa\n", pressure);
  printf("Mslp: %.2f mB\n", mslp);
  printf("Temp. = %.2fC\n", temperature / 100.0);
#endif

  battery_voltage = (( battery_level * BATTERY_ADC_FACTOR) - BATTERY_ADC_OFFSET ) * BATTERY_ADC_SCALE;
  
  
  rtc_get_datetime(&report_time);

  // format the output message

#ifdef DEBUG
  printf("... current wind direction : %.2f\n", radians_to_degrees(current_wind.direction));
#endif

  weatherReport sm;

  sm = format_weather_report(temperature,
                              humidity,
                              pressure,
                              mslp,
                              current_wind_5s, // Smooth out wind readings a little
                              max_gust,
                              averages2m,
                              gust10max,
                              rainToday,
                              rain_1h,
                              rainSinceLast,
                              battery_voltage);

  // next station message to rak11 format

  rainSinceLast = 0; // rain since last report (deprecated)

  rak811_change_state(WAKE);

  rak811_lorawan_put_hex((char *)&sm, sizeof(weatherReport), 100); // treat the structure as bytes
  
  rak811_change_state(SLEEP);
}

wind calc_average_wind(volatile const wind readings[], uint entries)
{
  /* Takes an array of structs of wind speeds (typdef wind) and directions 
     * and calculates an average speed (simple mean) and direction 
     * (a vector sum). This is my encoding of the method at 
     * www.noaa.gov/windav.shtml.
     * 
     * The basic method is to break down each direction and speed pair
     * into an east-west component and a north-south component. 
     * Directions are in Radians clockwise from north, so the 
     * north-south component is the cosine of the angle times the speed.
     * The east-west is the sine of the direction * speed. Then add up 
     * all the north-south components (NSc) and the east west (EWc) 
     * components (separately). The average direction is the arctan
     * of  (sum EWc / sum NSc). For reasons I don't understand, 
     * it's better to use the arctan2 call with sum of EWc as the first 
     * parameter, sum of NSc as the second (something to do with
     * less likely to be confused about quadrants. The second parameter
     * can't be zero, so make it 360, if that happens.
     * 
     */

#ifdef TRACE
  printf("> calc_average_wind\n");
#endif

  float NSc = 0; // Sum of north-south components
  float EWc = 0; // Sum of east-west components
  float sumSpeeds = 0;
  float avgDir = 0;
  wind averages = {0.0, 0.0};

  for (int x = 0; x < entries; x++)
  {
    /*#ifdef DEBUG
           printf("... x = %d, direction = %.2f, speed = %.2f\n",
                  x, readings[x].direction, readings[x].speed);
         #endif          
         */

    NSc += (cosf(readings[x].direction) * readings[x].speed);
    EWc += (sinf(readings[x].direction) * readings[x].speed);
    sumSpeeds += readings[x].speed;
  }
  /*#ifdef DEBUG
       printf("...sum NSc = %.2f\n", NSc);
       printf("...sum EWc = %.2f\n", EWc);
     #endif  
     */
  averages.speed = (sumSpeeds / entries);

  //if (0) {

  // atanf is meaningless if both components are zero

  printf("...sum NSc = %.2f\n", NSc);
  printf("...sum EWc = %.2f\n", EWc);
  if ((EWc == 0) && (NSc == 0))
  {
    avgDir = 0;
  }
  else
  {
    avgDir = atan2f(EWc, NSc);
  }
  printf("x\n");
  if (avgDir < 0)
    avgDir += (2 * PI); // Atan returns between - Pi
                        // and + Pi ( -180 to + 180 degrees)
  averages.direction = avgDir;

/*#ifdef DEBUG
       printf("...average wind direction: %.2fR\n", averages->direction);
       printf("...average wind speed: %.2f kph\n", averages->speed);
     #endif  
     */
#ifdef TRACE
  printf("< calc_average_wind\n");
#endif
  //}
  return averages;
}

void midnight_reset(void)
{
#ifdef TRACE
  printf("> midnight_reset\n");
#endif
  // Reset all the daily variables
  rainToday = 0;
  rainSinceLast = 0;
  max_gust.speed = 0;
  max_gust.direction = 0;

#ifdef TRACE
  printf("< midnight_reset\n");
#endif
}

void setup_arrays()
{
#ifdef DEBUG
  printf("> setup_arrays \n");
#endif

  for (int x = 0; x < 60; x++)
    rainHour[x] = 0;

  for (int x = 0; x < 120; x++)
  {
    windspeed[x].speed = 0;
    windspeed[x].direction = 0;
  }

  for (int x = 0; x < 10; x++)
  {
    gust10m[x].speed = 0;
    gust10m[x].direction = 0;
  }

#ifdef DEBUG
  printf("< setup_arrays \n");
#endif
}

// ==================== initialisation functions =======================

void set_real_time_clock()
{

#ifdef TRACE
  printf("> set_real_time_clock\n");
#endif

  rtc_init();

  // currently just setting this for development... will
  // be called from timesignal from UART
  //

  datetime_t t;

#ifdef DS3231

  i2c_init(I2C_PORT, 100 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  t = ds3231ReadTime();

#else

  t.year = DEFAULT_YYYY;
  t.month = DEFAULT_MON;
  t.day = DEFAULT_DD;
  t.dotw = DEFAULT_DOTW;
  t.hour = DEFAULT_HH;
  t.min = DEFAULT_MM;
  t.sec = DEFAULT_SS;

#endif

  rtc_set_datetime(&t);

#ifdef DEBUG
  printf("...Time set\n");
  printf("...RTC is: %i-%i-%i %02i:%02i:%02i\n",
         t.year,
         t.month,
         t.day,
         t.hour,
         t.min,
         t.sec);
#endif

#ifdef TRACE
  printf("< set_real_time_clock\n");
#endif
}

void setup_station_data()
{
#ifdef TRACE
  printf("> setup_station_data\n");
#endif
  // ==================== board ID first =========================//
  pico_unique_board_id_t board_id;

  char *p = stationdata.hardwareID; // The hex string version

  pico_get_unique_board_id(&board_id);

  memcpy(&stationdata.boardID,
         &board_id, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);

  //stationdata.boardID = board_id; // This is the binary version

  for (int x = 0; x < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; x++)
  {
    sprintf(p, "%02x", board_id.id[x]);
    p = p + 2; // above wrote 2 chars
  }
  *p = '\0'; // terminate the string

#ifdef DEBUG
  //printf("...unique identifier:");
  //for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
  //    printf(" %02x", board_id.id[i]);
  //}
  //printf("\n");

  printf("... station id: %s\n", stationdata.hardwareID);
#endif

  stationdata.timezone = 0;
  stationdata.altitude = 196; // for testing!
  stationdata.latitude = 0;
  stationdata.longitude = 0;
  stationdata.network_status = NETWORK_SYSTEM_BOOTING;

#ifdef TRACE
  printf("< setup_station_data\n");
#endif

  return;
}

/* Currently unused 
bool core0_led_timer_callback(struct repeating_timer *t)
{
  led_on(CORE0_LED);

  add_alarm_in_ms(LED_FLASH, core0_led_alarm_callback, NULL, false);
  return true;
}
int64_t core0_led_alarm_callback(alarm_id_t id, void *user_data)
{
  led_off(CORE0_LED);
  // Can return a value here in us to fire in the future
  return 0;
}
*/
void open_uart(void)
{

  uart_init(UART_ID, BAUD_RATE);
  uart_set_translate_crlf(UART_ID, false); // don't translate cr to lf

  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_set_hw_flow(UART_ID, false, false);
  uart_set_fifo_enabled(UART_ID, true);

  return;
}