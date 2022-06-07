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
#define UART_CHARACTER_DELAY 250 // in millionths/seconds. Tweak up if messages arive incomplete

#define CORE0_LED 14
#define RX_LED 15
#define BOOT_LED 15
#define TX_LED 13
#define BOOT_LED_FLASH 1000000 // How long to flash the boot LED
#define LED_FLASH 250 // how brief is the flash 1000000 = 1s
#define ERROR_FLASH 500 // Used to report an error 1000 = 1s
#define LED_GAP 200 // Gap between alternate flashes of two LEDS 1000=1s
#define CORE0_LED_INTERVAL 10000000 // every 10 seconds

#define RX_BUFFER_SIZE 550 // in bytes
#define TX_BUFFER_SIZE 300 // in bytes - review this

#define NETWORK_SYSTEM_BOOTING 0
#define NETWORK_UP 1
#define NETWORK_DOWN 2
#define NETWORK_INITIALISING 3
#define NETWORK_MODEM_ERROR 4
 
#define STATION_REPORT_DELAY 10000 // 1000 = 1s 

#define PI 3.14159265

#define DEBOUNCE 5000 // Number of microseconds IRQ must be after previous 

#define RAIN_CLICK 0.2794 // one click of the rain guage is .2794mm 

#define STATION_NAME_LENGTH 31
#define WINDCLICK 2.4 // 1 click per second = 2.4km/h
#define DELIM ";"

#define REPORTING_FREQUENCY 10 // in minutes

// Default start for RTC
#define DEFAULT_YYYY 2000
#define DEFAULT_MON 01
#define DEFAULT_DD 01
#define DEFAULT_DOTW 0
#define DEFAULT_HH 23
#define DEFAULT_MM 54
#define DEFAULT_SS 50

#define CORE1_SLEEP_CYCLE 250 // In milliseconds. 

#define DEFAULT_DECIMAL_PLACES 2

#define TIME_SYNC_DELAY 0000000 // Time sync messages need to allow for the delay
                                // in the message getting from the the basestation to the 
								// UART interrupt on the pico.
								// This needs tweaking by experiment (in millionths of a second)

#endif
