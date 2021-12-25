#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/unique_id.h" 
#include "openaws.h"
#include "types.h"
#include "constants.h"

#include "uart_lora.h"
#include "rak811.h"

#define DEBUG
#define TRACE
//#define DEBUG_ISR
//#define TRACE_ISR

/* ================== recognised messages ============================
 * All this code needs rewriting for binary messages !
 * 
 * 
 * 
 * 200 - set rtc (time)
 *  
 * "200;e6605481db318236;2021;02;23;5;17;23;00;Z;\r\n"
 *   ^       ^             ^   ^  ^ ^  ^  ^  ^ ^
 *  Code    hardware id   YYYY MM DD^ HH MM SS Timezone
 *                                  ^
 *                                  Day of week     
 * 
 * Day of week: Sunday = 0
 * Timezone is in ISO 8601 notation ie Z or +01:00 etc
 *                -------------------------------
 * 
 * 201 - set station data
 * 
 * 201;e6605481db318236; Curbar AWS;53.27044,-1.62351;180;
 *  ^         ^                ^          ^             ^
 * Code   hardware id    station name  lat/long      altitude in m
 */

extern stationData stationdata;

extern uint8_t RX_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t * RX_buffer_pointer; 

extern volatile uint64_t UART_RX_interrupt_time;

void process_RX_message (const char * buffer) {
    /*Takes an input message - 
    * checks for type and that the ID matches the station
    */
	
    #ifdef TRACE
      printf("> process_RX_message\n");
    #endif 
    
    char buffer_copy [RX_BUFFER_SIZE]; // strtok breaks the original
    char * type; 
    char * hardwareID;
		
    strcpy(buffer_copy, buffer);
    
      
    /* The RAK811 returns data in the form at+recv=-69,6,113:3130303B...;
     * so it needs some extra processing.
     */ 
    
    #ifdef RAK811
      rak811_format_recvd (buffer_copy);
    #endif  
	
    // parsing message - first is the type of message, then the hardware id
	
    type = strtok(buffer_copy, DELIM);
    hardwareID = strtok(NULL, DELIM);
	
    #ifdef DEBUG
	printf("...incoming message - type: %s, hardwareID: %s\n", type, hardwareID);
    #endif  
	
    int type_i = atoi(type);
	
    switch(type_i) {
	case 200 :
	    set_rtc_time(buffer_copy, &stationdata);
	    break;
        case 201 :
	    set_station_data(buffer_copy, &stationdata);
	    break;
	case 202 :
	    report_station_details();
	default :
	    printf("Invalid type %d,n", type_i);  
	 
     }    
	 
    #ifdef TRACE
      printf("< process_RX_message\n");
    #endif 
}

void set_rtc_time(char * buffer, stationData * stationdata) {
	
    #ifdef TRACE
      printf("> set_rtc_time\n");
    #endif 
    
    char * yyyy, * mo, * dd, * day, * hh, * mm, * ss, * tz;
    
    // "200;e6605481db318236;2021;02;23;5;17;23;00;+1:00;\r\n"
    yyyy = strtok(NULL, DELIM);
    mo = strtok(NULL, DELIM);
    dd = strtok(NULL, DELIM);
    day = strtok(NULL, DELIM);
    hh = strtok(NULL, DELIM);
    mm = strtok(NULL, DELIM);
    ss = strtok(NULL, DELIM);
    
    tz = strtok(NULL, DELIM);
    
    //datetime_t t;
    //t.year = 2020;
       
    datetime_t t = {.year = atoi(yyyy), 
	            .month = atoi(mo), 
		    .day = atoi(dd), 
		    .dotw = atoi(day),
		    .hour = atoi(hh),
		    .min = atoi(mm),
		    .sec = atoi(ss) }; 
		    
    //strcpy(stationdata->timezone,tz); 
    
    rtc_set_datetime (&t);
    #ifdef DEBUG
      printf("...Time set\n");
      printf("...RTC is: %i-%i-%i %02i:%02i:%02i Timezone: %s\n", 
              t.year,
              t.month,
              t.day,
              t.hour,
              t.min,
              t.sec,
	      stationdata->timezone);  
    #endif
    
    #ifdef TRACE
      printf("< set_rtc_time\n");
    #endif 
}

void set_station_data(char * buffer, stationData * stationdata) {
    #ifdef TRACE
      printf("> set_station_data\n");
    #endif 
    //201;e6605481db318236;Curbar AWS;53.27044,-1.62351;180;
    
    char * name, * position, * altitude;
    
    name = strtok(NULL, DELIM);
    position = strtok(NULL, DELIM);
    altitude = strtok(NULL, DELIM);
    
    strcpy(stationdata->name, name);
    strcpy(stationdata->position, position);
    stationdata->altitude = atoi(altitude);
    
    #ifdef DEBUG
      printf("... station data changed - name: %s, position: %s, altitude: %d\n", 
              stationdata->name, stationdata->position, stationdata->altitude);
    #endif
    	       
    #ifdef TRACE
      printf("< set_station_data\n");
    #endif 
}

void report_station_details(void) {
	#ifdef TRACE
      printf("< sreport_station_details\n");
    #endif
    
    #ifdef TRACE
      printf("< report_station_details\n");
    #endif
}
	
void open_uart(void) {
    
    #ifdef TRACE
     printf("> open_uart\n");
    #endif 
    
    RX_buffer_pointer = RX_buffer; // inittialise the buffer
                                   // updated on interrupt
				   
    memset (RX_buffer,'\0', RX_BUFFER_SIZE);
				   
    uart_init(UART_ID, BAUD_RATE);
    uart_set_translate_crlf (UART_ID, false); 	 // don't translate cr to lf

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    
    // Set up a RX interrupt
    
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Enable the UART to send interrupts - RX only
    
    UART_RX_interrupt_time = 0;
    uart_set_irq_enables(UART_ID, true, false);
    
    #ifdef TRACE
       printf("< open_uart\n");
    #endif 
    
    return;
}

void on_uart_rx() {
        
    #ifdef TRACE_ISR
     printf("> on_uart_rx\n");
    #endif 
    /* Be careful with the DEBUG and TRACE below, they slow down the 
     * ISR with some odd effects
     */
     
    // if interrupt time is 0 - treat this as a new message
    if (UART_RX_interrupt_time == 0) UART_RX_interrupt_time = time_us_32();
    
    int chars_rx = 0;
             
    while (uart_is_readable(UART_ID)) {
	*RX_buffer_pointer = uart_getc(UART_ID);
        //sleep_ms(10);
        RX_buffer_pointer++;
        chars_rx++;
        if (chars_rx >= RX_BUFFER_SIZE){
	    printf("...ERROR: Input buffer overload\n");
            assert(chars_rx >= RX_BUFFER_SIZE);
            break;
	}
        //asm volatile("nop \n nop \n nop"); // slow down a touch
    }
             
    // Terminate the string - just in case 
    //*(RX_buffer_pointer + 2) = 0;
                
    #ifdef DEBUG_ISR
     printf("...message received:%s\n", RX_buffer);
    #endif         
            
    #ifdef TRACE_ISR
     printf("< on_uart_rx\n");
    #endif 
    
    return;
}         
	
	 
	
