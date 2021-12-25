#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// GPIO PINS
#define WIND_IRQ 2  // GPIO pin 2 = pin 4 on board
#define RAIN_IRQ 3  // GPIO pin 3 = pin 5 on board
#define WIND_DIR 0  // ADC0 = Pin 31 GPIO pin 26 (below)
#define WIND_DIR_GP 26

#define HARDWARE_ID_LENGTH 17 // or PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1 byte

// UART

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define CORE0_LED 14
#define CORE1_LED 13
#define BOOT_LED 15
#define BOOT_LED_FLASH 1000000 // How long to flash the boot LED
#define LED_FLASH 1000 // how brief is the flash 1million = 1s
#define LED_INTERVAL 5000000 // every 5 seconds

#define RX_BUFFER_SIZE 550 // in bytes
#define TX_BUFFER_SIZE 300 // in bytes - review this

#define PI 3.14159265

#define DEBOUNCE 5000 // Number of microseconds IRQ must be after previous 

#define RAIN_CLICK 0.2794 // one click of the rain guage is .2794mm 

#define STATION_NAME_LENGTH 31
#define WINDCLICK 2.4 // 1 click per second = 2.4km/h
#define DELIM ";"

#define REPORTING_FREQUENCY 5 // in minutes

// Default start for RTC
#define DEFAULT_YYYY 2000
#define DEFAULT_MON 01
#define DEFAULT_DD 01
#define DEFAULT_DOTW 0
#define DEFAULT_HH 23
#define DEFAULT_MM 54
#define DEFAULT_SS 50

#define CORE1_SLEEP_CYCLE 60000 // In milliseconds. Also frequency of 
								// pressure/temp/humidity readings
								// which get done each cycle


#define DEFAULT_DECIMAL_PLACES 2

#endif
