# Openaws : an Open Automatic Weather Station on Raspberry Pi Pico
## Installation
The bit where you are hoping for some helpful installation instructions. At the moment you are on your own. Compile it and get the debugger ready.
## Information
This is not an out-the-box solution. You are going to need to do a bit of work to get up and running. A soldering iron is essential and there will be problems to solve. The below is as much for my benefit as anyone - so I can remember how this works next time it breaks.
*Tom Cooper*
January 2022

## Overview
The code is a functioning weather station Pico 'head end' deisgned to report wind speeds and direction, rainfall, barometric pressure, humidity and temperature at a chosen report interval.
The station also keep a handful of daily totals (such as rainfall).
Communication is two way with the weather station sending out reports and processing, for example, time zone mesages from the base station (more on the basestation below).
The system is designed to communicate with a loraWan network such as the Things Network. 
The latest iteration of the code relies on a DS3231 clock. Development was done with a Wavesure, pico-RTC-DS3231

## Wiring and hardware
### Pico

#### Connections

Note the GND connections could be to any of the pico ground pins

| Pin number | Pico name     | Connection | Notes | Constant.h value (if used) |
| ----- | ----     | ---------- | ----- | ------------------ |
| 1     | UART0 TX | to RAK811 RX | Direct connection |  |
| 2     | UART0 RX | to RAK811 TX | Direct connection |  |
| 3     | GND      | general earth      | Used as a ground for the wind sensor etc | |
| 4     | GP2      | to Wind      | Pin pulled up and interrupt fired when connected to GND via 1kohm | WINDIRQ|
| 5     | GP3      | to Rain | Pin pulled up and interrupt fired when connected to GND via 1kohm | RAINIRQ |
| ... |
| 17    | GP13     | to status LED |Via appropriate resistor say 330 ohm, flashed on message send. Also flashed with RX_LED to indicate receive with an unknown hardware key. | TX_LED |
| 18    | GND      | used for LEDs | | 
| 19    | GP14     | to status LED |flashed every CORE0_LED_INTERVAL microseconds. Also double flashed to confirm a successfully decoded incoming message | CORE0_LED |
| 20    | GP15     | to status LED |flashed on boot for BOOT_LED_FLASH microseconds. Also on message receive. Flashed with TX_LED to indicate receive with an unknown hardware key. | BOOT_LED RX_LED|
| 21    | SPIO RX  | to BME280 | Pin names vary on sensor boards - look for SDO | |
| 22    | SPIO CSn | to BME280 | Typically CSB/CS - chip select | |
| 23    | GND      | to BME280 | To ground on board | |
| 24    | SPIO SCK | to BME280 | SCL/SCK | |
| 25    | SPIO TX  | to BME280 | SDA/SDI | |
| ... |
| 31    | ADCO     | Wind direction | Connected via voltage splitter to weather meters - see notes below | WIND_DIR|
| ... |
| 36    | 3V3 OUT  | various  |Source of power for RAK811, BME280 and ADC voltage | |
| 38    | GND      |  various | General ground for RAK811, IRQs etc| | 

#### Voltage splitter

#### Sparkfun weather meters
 
### Rak811 - lora
#### Wiring
#### Lora setup
Set the RAK811 with the appropriate setting for your loraWan network before you start - this isn't done from this code. 

##### Some useful AT commands for RAK811


at+get_config=lora:status

Airtime calculator https://avbentem.github.io/airtime-calculator/ttn/eu868/49

#### Clock setup
Set the DS3231 to UTC before you start - this isn't done in the code.


## Pico processing and design
Openaws is coded in C using the standard Pico SDK kit. The code uses the pico's real time clock (RTC) plus various timer interrupts and alarms to collect data and report the weather.
The design uses core0 to handle the communication links, generate the various outgoing messages and process any incoming ones. core0 also reads the pressure/temperature/humidity enor as part of the reporting process. core1 handles the rain gauge and wind speed interrupts, and the wind direction ADC.
### core0
Weather reports (message type 100) are generated at REPORTING_FREQUENCY minute intervals. Note this should be a divisor of 60. The reports are triggered by a repeating alarm set on the pico's real time clock (RTC). The clock is set to a default time on startup. It is synced with the basestation when it receives a 201 message type (below). 
Station details (including the altitude setting important for correct pressure calculation) are also defaulted but need setting up with a 200 message (below).
Various totals are reset at midnight, local time, using the RTC. The RTC is set to UTC, but a timezone is recorded in the station data to adjust this. The data messages use an eight-byth epoch UTC time. (Beware the 2034 bug). 

The pressure/temperature/humidity sensor (currently BME280) is read as part of the weather report process. There is an SPI connection to the **BME280 sensor** At the moment the sensor code has sleeps in it so it cannot be run via a timer. 
#### uart handling -p2p
The code is set up to communicate with the RAK811 via 'at' codes. The data arrives from (and is sent to) the RAK811 as hex encoded characters eg, a typical weather report is sent as: 
``` 
at+send=lora:2:640000000B4000001E000B4000000000000000012D107160000007000000001868D00018ED5\r\n 
``` 
This data will be received via a loraWAN gateway and will be transferred to (in my case) the Things Network. The basestation code will pick up the data from there and update a local database.

**Incoming Mesages** are receivable for a brief period after transmission. The RAK811 is configured as a Class A loraWan device. 


**Outgoing messages** is largely about coercing C variable into hex-byte arrays and stuffing them at the 
RAK811.

#### Weather data
All the weather data is being recorded over in core1. This is kept in global variables. The pico does have some proper fifo inter-core communication mechanisms. Maybe one day... but in the meantime, we do not seem to get any problems of variable being read whilst updating. 
### core1
Life is simple over in core1. A one second repeating timer updates arrays of wind and rainfall data. To allow generation of, for example rain in the past hour, and 2 minute wind averages.

**Rain** is recoded via a gpio interrupt callback. Each click of the tipping gauge is RAIN_CLICK mms of rain. 

**Wind speed** operates via a similar interrupt with one click per second representing WINDCLICK km/h. **Wind direction** is via an ADC. This is kept in radians to assist in calculating average directions (over in core 0).
The only other trick is to make sure the various array indexes are updated correctly so clicks are recorded in the right array position. 

### History
My original weather station was developed on an Arduino using the Sparkfun weather shield, weather meters and the code supplied by Sparkfun. I added a serial data link to this (instead of WiFi) and this has served well for 8+ years. Over that time I have fiddled a lot with the code and changed much hardware as various serial links failed or became obsolete. 

The launch of the pico gave me a chance to start from scratch (with several years' more experience). This was also an opportunity to assess if LoRa would pprovide a suitable (future proof?) data links. I am also more confident working in C on microcontrollers rather than Arduino's C++ dialect.

The use of the pico allows the use of an onboard real time clock, plus many of the timer and alarm features on the pico which are not available on the Arduino.

## Message types and handling
Note this is a Class A loraWAN device. Incoming messages (downlinks, in loraWAN speak), are received in a brief period following an uplink. Uplinks only happen every REPORTING_FREQUENCY minutes (I use 10mins), so you will have to wait for responses.
### Currently implemented message types

| Message number | Hex | Details | Direction |
| -------------- | --- | ------- | --------- |
| 100            | x64 | Contains the latest weather readings | Uplink weather station > loraWAN |
| 101            | x65 | Details of station including date, time, altitude and position | Uplink |
| .. |
| 200            | xc8 | Set time zone - sends the current base station timezone to weather station | Downlink loraWAN > weather station |
| 201            | xc9 | Set station details - sends the station details - altitude, position - to the weather tation |  Downlink |
| 202            | xca | Request station details - sends a request for the weather station to send a message 101 |  Downlink |
| 203            | xcb | Request softwarre reset - asks the station to do a software reset (via watchdog) |  Downlink |

## Interaction with loraWAN
This is fairly basic. When the station boots, it tries to join the network with an 'at+join' command. If this fails (either bad parameters or no gateway available) it is retried RAK811_JOIN_ATTEMPTS number of times. If no connection is made, the network status is recorded as NETWORK_DOWN in the station data. A join will be retried the next time a weather report is to be sent.

Note the RAK811 should have been set up with the appropriate loraWAN parameters - device IDS etc - before connecting up. I do this with a serial terminal. See below.

**Outgoing mesages** (uplinks) are currently just the regular weather reports (100) and a confirmation message of the station details (101). When it is time for one of these messages, the network status is checked and an attempt to join is made, if NETWORK_DOWN. The message is put to the RAK811 serial port ('at+send=lora:2:6400...'). The response from the modem is read (hopefully 'OK'), but possibly an error code eg 'ERROR 99'. Error handling is limited at the moment but this does not appear to be an issue in live running.

After 'OK' is received, the station keeps checking the UART for RAK811_DOWNLINK_WAIT milliseconds. This is to pick up any **incoming messages** (downlinks) from the network. These are processed as per the table above. At the moment, try not to send more than one downlink per reporting cycle - for example if you have sent a set sttation details message (201), wait until the next reporting cycle to send a timezone (200) message.

### Set up sequence for loraWAN on RAK811 ###

at+set_config=lora:work_mode:0 # Switch to lowrawan mode

at+set_config=lora:join_mode:0 # OTAA activation

at+set_config=lora:class:0 # class A

at+set_config=lora:region:EU868

at+set_config=lora:dev_eui:nnnnnnnnnnnnnnnn

at+set_config=lora:app_eui:nnnnnnnnnnnnnnnn

at+set_config=lora:app_key:<32 characters> 

at+set_config=device:restart

at+get_config=lora:status # to check...

at+join # Connect to lorawan - may take a few tries (up to 5)

at+send=lora:2:1234567890ABFF # try sending some data

## Code structure

| Module | Description | List of procedures | Notes |
| ------ | ----------- | -------------------| ----- | 
| openaws.c| Initialisation and control | void setup_arrays(void) | |
|          |                            | void setup_station_data(void)|  |
|          |                            | void set_real_time_clock(void) | |
|          |                            | void open_uart(void) | |
|          |                            | void launch_core1(void) | |
|          |                            | int get_sector (uint16_t adc) |  |
|          |                            | wind calc_average_wind(volatile const wind readings[], uint entries) | |
|          |                            | float radians_to_degrees (float radians) | |
|          |                            | void midnight_reset (void) | |
|          |                            | void report_weather(int32_t humidity, int32_t pressure, int32_t temperature) | |
|          |                            | float adjust_pressure (int32_t pressure, int32_t altitude, float temperature) | |
|          |                            | void minute_processing (void) | | 
| bme280.c | Initialise and control sensor| | |
| core1_processing.c | Collects weather readings | void core1_process (void) | Entry point | 
|          |                            |
|          |                            |
|          |                            |
|          |                            |

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