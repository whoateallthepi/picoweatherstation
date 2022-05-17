#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"

#include "ds3231.h"

#define BCD_TO_INT(x) (x - (6 * (x >> 4)))

datetime_t ds3231ReadTime()
{
    uint8_t val = 0x00;
    uint8_t addr = 0x68;

    //regaddr,seconds,minutes,hours,weekdays,days,months,years
    char buf[] = {0x00, 0x50, 0x59, 0x17, 0x04, 0x05, 0x03, 0x21};

    i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
    i2c_read_blocking(I2C_PORT, addr, buf, 7, false);

    buf[0] = buf[0] & 0x7F; //sec
    buf[1] = buf[1] & 0x7F; //min
    buf[2] = buf[2] & 0x3F; //hour
    buf[3] = buf[3] & 0x07; //week
    buf[4] = buf[4] & 0x3F; //day
    buf[5] = buf[5] & 0x1F; //month

    datetime_t ds3231_datetime = {.year = (2000 + ( BCD_TO_INT (buf[6]))),
                                  .month = BCD_TO_INT(buf[5]), 
                                  .day = BCD_TO_INT(buf[4]),
                                  .dotw = (BCD_TO_INT(buf[3]) - 1), // pico is 0-6 (0=Sun) ds3231 is 1-7, treat 1 as Sunday
                                  .hour = BCD_TO_INT(buf[2]),
                                  .min = BCD_TO_INT(buf[1]),
                                  .sec = BCD_TO_INT(buf[0])};

    return ds3231_datetime;
    
}
