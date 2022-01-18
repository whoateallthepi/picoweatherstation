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
In the initial versons the communications is handled by a LORA peer to peer link. This is done via a pair of RAK811 units (more on those below).
Communication is two way with the weather station sending out reports and processing, for example, time sync mesages from the base station (more on the basestation below).
In the future we may have more LoRaWAN integration.

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
| 19    | GP14     | to status LED |flashed every CORE0_LED_INTERVAL microseconds | CORE0_LED |
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
Set the RAK811 frequencies and p2p mode from a serial terminal before you start - this isn't done from this code. 

## Pico processing and design
Openaws is coded in C using the standard Pico SDK kit. The code uses the pico's real time clock (RTC) plus various timer interrupts and alarms to collect data and report the weather.
The design uses core0 to handle the communication links, generate the various outgoing messages and process any incoming ones. core1 reads the various sensors.
The pico hardware key is encoded on the incoming and outgoing messages (for identification). This should be checked on incoming messages too! Currently there is no security on the incoming mesages - this needs implementing for a 'production' system.
### core0
Weather reports (message type 100) are generated at REPORTING_FREQUENCY minute intervals. Note this should be a divisor of 60. The reports are triggered by a repeating alarm set on the pico's real time clock (RTC). The clock is set to a default time on startup. It is synced with the basestation when it receives a 201 message type (below). 
Station details (including the altitude setting important for correct pressure calculation) are also defauted but need setting up with a 200 message (below).
Various totals are reset at midnight on the RTC. Note the RTC is set to local time but the data messages use an eight-byth epoch UTC time. (Beware the 2034 bug). So the station data includes a timezone which has to be incorporated into the UTC time conversion. 
#### uart handling
The code is set up to communicate with the RAK811 via at codes. For development you can #undef RAK811 and jut get serial output to a terminal etc. Although much of that alternative code has been untried for most of the development cycle. The data arrives from (and is sent to) the RAK811 as hex encoded characters eg, a typical weather report is sent as: 
``` 
at+send=lorap2p:64E6605481DB31823661DC78D0000000B4000001E000B4000000000000000012D107160000007000000001868D00018ED5\r\n 
``` 
 This is received at the base station  as 
```
at+recv=-30,7,49:64E6605481DB31823661DC78D0000000B4000001E000B4000000000000000012D107160000007000000001868D00018ED5
```

The biggest headache is getting the **incoming messages** working smoothly. This is still a work in progress (as of January 2022). Part of the issue seems to be the relatively slow speed of characters arriving from the RAK811 buffer (slow compared to the pico, that is). 

The current operation is as follows: an interrupt is set on the pico uart. When this is triggered, an interupt time is stored in global UART_RX_interrupt_time and characters are copied to RX_Buffer (another global).
The main loop checks UART_RX_interrupt_time but doesn't start processing incoming messages until RAK811_RX_DELAY microseconds have passed. I've been using 1 second. But if you are getting incomplete messages, try tweaking that.
The RAK811 sequences (at commands) are documented in the [RAK811 AT Command Manual](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK811-Module/AT-Command-Manual/)

Without a delay, partial messages were common. The fifo on the pico uart is enabled ``` uart_set_fifo_enabled(UART_ID, true); ``` in open_uart - see uart_lora.c. But the buffer on the pico is small so some characters have to be read from the buffer before the rest of the message arrives.

A relative degree of certainty has been reached by trying various delays in the uart irq and trying fifo on and off. But processing of incoming messages is far from perfect and needs more work. 

 
**Outgoing messages** are a relative breeze. Setting these up is largely about coercing C variable into hex-byte arrays and stuffing them at the 
RAK811. The RAK811 is generally in 'receive' mode (at+set_config=lorap2p:transfer_mode:1) but this is switched to transfer_mode:2 for sending. This isn't full duplex, but incoming messages should be rare (once a day at most). So we just wear any problems.
#### Weather data
All the weather data is being recorded over in core1. This is kept in global variables. The pico does have some proper fifo inter-core communication mechanisms. Maybe one day... but in the meantime, we do not seem to get any problems of variable being read whilst updating. 
### core1
Life is simpler over in core1. There is an SPI connection to the **BME280 sensor** (read every second via the main loop.) At the moment the sensor code has sleeps in it so it cannot be run via a timer. A one second repeating timer updates arrays of wind and rainfall data. To allow generation of, for example rain in the past hour, and 2 minute wind averages.

**Rain** is recoded via a gpio interrupt callback. Each click of the tipping gauge is RAIN_CLICK mms of rain. 

**Wind speed** operates via a similar interrupt with one click per second representing WINDCLICK km/h. **Wind direction** is via an ADC. This is kept in radians to assist in calculating average directions (over in core 0).
The only other trick is to make sure the various array indexes are updated correctly so clicks are recorded in the right array position. 

### History
My original weather station was developed on an Arduino using the Sparkfun weather shield, weather meters and the code supplied by Sparkfun. I added a serial data link to this (instead of WiFi) and this has served well for 10+ years. Over that time I have fiddled a lot with the code and changed much hardware as various serial links failed or became obsolete. 

The launch of the pico gave me a chance to start from scratch (with 10 years' more experience). This was also an opportunity to assess if LoRa would pprovide a suitable (future proof?) data links. I am also more confident working in C on microcontrollers rather than Arduino's C++ dialect.

The use of the pico allows the use of an onboard real time clock, plus many of the timer and alarm features on the pico which are not available on the Arduino.

## Message types and handling
## Interaction with basetation
## Code structure