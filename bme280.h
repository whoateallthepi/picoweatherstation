#ifndef BME280_H_
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define BME280_H

/* Pins:

   GPIO 16 (pin 21) MISO/spi0_rx-> SDO/SDO on bme280 board
   GPIO 17 (pin 22) Chip select -> CSB/!CS on bme280 board
   GPIO 18 (pin 24) SCK/spi0_sclk -> SCL/SCK on bme280 board
   GPIO 19 (pin 25) MOSI/spi0_tx -> SDA/SDI on bme280 board
   3.3v (pin 3;6) -> VCC on bme280 board
   GND (pin 38)  -> GND on bme280 board

   Register definitions, and compensation code from the Bosch datasheet 
   https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
*/
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define SPI_PORT spi0
#define READ_BIT 0x80

// ===================function prototypes===============================

int bme280_initialise (void);
int bme280_fetch (int32_t * humidity, int32_t * pressure, int32_t  * temperature); 

// below functions are for internal use only
int32_t compensate_temp(int32_t adc_T);
uint32_t compensate_pressure(int32_t adc_P);
uint32_t compensate_humidity(int32_t adc_H);
static inline void cs_select(void);
static inline void cs_deselect(void);
static void write_register(uint8_t reg, uint8_t data);
static void read_registers(uint8_t reg, uint8_t *buf, uint16_t len);
void read_compensation_parameters(void);
static void bme280_read_raw(int32_t *humidity, 
    int32_t *pressure, int32_t *temperature); 

#endif
