
#include "utilities.h"
#include <math.h>
#include <time.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "constants.h"

void software_reset()
{
    watchdog_enable(1, 1);
    while(1);
}

int float_to_int(float in, float decimal_places)
{
    /* Takes a floating point number and returns an int with the required number of
decimal places incorporated 
eg float_to_int(37.6666,2) returns 3767 
works with negative numbers and should work with negative decimal places - if you really 
want
*/
    float shifted;
    float factor = powf(10, decimal_places);

    if (in >= 0)
    {
        shifted = (int)(in * factor + .5);
    }
    else
    {
        shifted = (int)(in * factor - .5);
    }

    return shifted;
}

int32_t float_to_int32(float in, float decimal_places)
{
    /* Takes a floating point number and returns an int with the required number of
decimal places incorporated 
eg float_to_int(37.6666,2) returns 3767 
works with negative numbers and should work with negative decimal places - if you really 
want
*/
    float shifted;
    float factor = powf(10, decimal_places);

    if (in >= 0)
    {
        shifted = (int)(in * factor + .5);
    }
    else
    {
        shifted = (int)(in * factor - .5);
    }

    return shifted;
}

time_t rtc_to_epoch(datetime_t rtc, int8_t timezone)
{
    struct tm t;
    t.tm_year = (rtc.year - 1900);
    t.tm_mon = (rtc.month - 1); //  0 = jan
    t.tm_mday = rtc.day;
    t.tm_hour = rtc.hour;
    t.tm_min = rtc.min;
    t.tm_sec = rtc.sec;
    t.tm_isdst = timezone;

    return mktime(&t);
}

void epoch_to_rtc(time_t epoch, datetime_t *rtc)
{

    struct tm t;

    t = *localtime(&epoch);

    rtc->year = (t.tm_year + 1900);
    rtc->month = (t.tm_mon + 1); //  1 = jan
    rtc->day = t.tm_mday;
    rtc->hour = t.tm_hour;
    rtc->min = t.tm_min;
    rtc->sec = t.tm_sec;
    rtc->dotw = t.tm_wday;
}
float radians_to_degrees(float radians)
{
#ifdef TRACE
    printf("> radians_to_degrees\n");
#endif

    return (radians * (180 / PI));

#ifdef TRACE
    printf("< radians_to_degrees\n");
#endif
}

int32_t hex2int32(char *hex)
{
    // takes 8 hex digit and convert to an int32
    // currently no validation

    uint32_t value = 0;

    for (int x = 0; x < 8; x++)
    {
        value <<= 4; // won't matter the firt time thru
        value += getNum(*(hex + x));
    }
    return value;
}

int16_t hex2int16(char *hex)
{
    // takes 4 hex digits and convert to an int16

    uint16_t value = 0;

    for (int x = 0; x < 4; x++)
    {
        value <<= 4; // won't matter the firt time thru
        value += getNum(*(hex + x));
    }
    return value;
}

int hex2int(char *hex)
{
    // takes 2 hex digits and convert to an int
    int value = 0;
    value = getNum(*hex);
    value <<= 4;
    value += getNum(*(hex + 1));
    return value;
}

int hex2int2sComplement(char *hex)
{
    // takes 2 hex digits and convert to an int
    int value = 0;
    value = getNum(*hex);
    value <<= 4;
    value += getNum(*(hex + 1));
    if (value > 127) return (value - 256); else return value; 
}

int getNum(char ch)
{
    // get the numeric value of a hex byte
    int num = 0;
    if (ch >= '0' && ch <= '9')
    {
        num = ch - 0x30;
    }
    else
    {
        switch (ch)
        {
        case 'A':
        case 'a':
            num = 10;
            break;
        case 'B':
        case 'b':
            num = 11;
            break;
        case 'C':
        case 'c':
            num = 12;
            break;
        case 'D':
        case 'd':
            num = 13;
            break;
        case 'E':
        case 'e':
            num = 14;
            break;
        case 'F':
        case 'f':
            num = 15;
            break;
        default:
            num = 0;
        }
    }
    return num;
}


int int2BCD(int x) {
    int a,b;
    a = x / 10; // gives us first BCD nibble 
    b = x % 10; // other nibble
    return ( (16 * a) + b);
}




void setup_led(uint led)
{
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);
    gpio_put(led, 0);
    return;
}

void led_on(uint led)
{
    gpio_put(led, 1);
    return;
}

void led_off(uint led)
{
    gpio_put(led, 0);
    return;
}

void led_flash(uint led)
{
    led_off(led);
    led_on(led);
    busy_wait_ms(ERROR_FLASH);
    led_off(led);
}


void led_double_flash(uint led)
{
    led_off(led);
    led_on(led);
    busy_wait_ms(ERROR_FLASH);
    led_off(led);
    busy_wait_ms(100);
    led_on(led);
    busy_wait_ms(ERROR_FLASH);
    led_off(led);
}

void led_repeat_flash(uint led)
{

    while (1)
    {
        led_on(led);
        busy_wait_ms(ERROR_FLASH);
        led_off(led);
        busy_wait_ms(ERROR_FLASH / 2);
    }
}

void led_repeat_two_leds_alternate(uint led1, uint led2)
{
    led_off(led1);
    led_off(led2);

    while (1)
    {
        led_on(led1);
        busy_wait_ms(ERROR_FLASH / 2);
        led_off(led1);
        busy_wait_ms(LED_GAP);
        led_on(led2);
        busy_wait_ms(ERROR_FLASH / 2);
        led_off(led2);
        busy_wait_ms(LED_GAP);
    }
}

void led_flash2_leds (uint led1, uint led2)
{
    led_on(led1);
    led_on(led2);
    busy_wait_ms(LED_FLASH);
    led_off(led1);
    led_off(led2);
}

void led_flash3_leds (uint led1, uint led2, uint led3)
{
    led_on(led1);
    led_on(led2);
    led_on(led3);
    busy_wait_ms(LED_FLASH);
    led_off(led1);
    led_off(led2);
    led_off(led3);
}

int bytes_compare(const char *bytes1, const char *bytes2, const int bytes)
{
    for (int x = 0; x < bytes; x++)
    {
        if (bytes1[x] != bytes2[x])
            return 0;
    }
    return 1;
}

float adjust_pressure(int32_t pressure, int32_t altitude, float temperature)
// Adjusts a barametric pressure to sea level
{
  float pressure_mb;
  float pressure_corrected;

#ifdef TRACE
  printf("> adjust_pressure\n");
#endif

  // Lifted from Nathan Seidle's C code for the Electric Imp - thanks

  pressure_mb = pressure / 100; // pressure is now in millibars
  /*
    part1 = pressure_mb - 0.3 # Part 1 of formula
    part2 = 8.42288 / 100000.0
    part3 = math.pow((pressure_mb - 0.3), 0.190284);
    part4 = altitude / part3
    part5 = (1.0 + (part2 * part4))
    part6 = math.pow(part5, (1.0/0.190284))
    bar_corrected = part1 * part6 # Output is now in adjusted millibars
    */
  pressure_corrected = pressure_mb + ((pressure_mb * 9.80665 * altitude) / (287 * (273 + temperature + altitude / 400)));

/* # lifted from :  https://gist.github.com/cubapp/ (information only)

    in standard places (hasl from 100-800 m, temperature from -10 to 35) 
    is the coeficient something close to hasl/10, meaning simply 
    a2ts is about  aap + hasl/10
    bar_corrected = pressure_mb / pow(1.0 - altitude/44330.0, 5.255)
    */
#ifdef TRACE
  printf("< adjust_pressure\n");
#endif
  return pressure_corrected;
}
