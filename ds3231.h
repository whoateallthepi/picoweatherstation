#ifndef __DS3231_H
#define __DS3231_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
 
// use i2c0(GP20,GP21) 
#define I2C_PORT	i2c0
#define I2C_SCL		20	
#define I2C_SDA		21


datetime_t ds3231ReadTime();
void ds3231SetTime(datetime_t t); 

#endif