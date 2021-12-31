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
#include "utilities.h"

//#define DEBUG
//#define TRACE
//#define DEBUG_ISR
//#define TRACE_ISR

/* ================== recognised messages ============================
 * All this code needs rewriting for binary messages !
 * 
 * 
 * 
 * 200 - set rtc (time)
 *  
 * "c8|e6605481db318236|38700f84|ffff\r\n"
 *   ^       ^             ^       ^
 *  Code    hardware id   epoch   Timezone
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
extern volatile uint8_t *RX_buffer_pointer;

extern volatile uint32_t UART_RX_interrupt_time;

void process_RX_message(char *buffer)
{
  /*Takes an input message - 
    * checks for type and that the ID matches the station
    */

#ifdef TRACE
  printf("> process_RX_message\n");
#endif

  incomingMessage im; // will contain all the data from the message
  int chars_received = 0;
  int32_t rssi = 0; // Signal strength and signal/noise ratio from modem
  int32_t snr;
  int type_i; // will hold mesage tye as an int

  chars_received = rak811_process_incoming(buffer, &im, &rssi, &snr);

  // get the message_type as an int so we can switch it

  // should validate the hardware key here - skipping for testing

  type_i = hex2int(im.messagetype);

  switch (type_i)
  {
  case 200:
    set_rtc_time(&im);
    break;
  case 201:
    //set_station_data(buffer_copy, &stationdata);
    break;
  case 202:
    //report_station_details();
  default:
    printf("Invalid type %d,n", type_i);
  }

#ifdef TRACE
  printf("< process_RX_message\n");
#endif
}

void set_rtc_time(incomingMessage *data)
{
#ifdef TRACE
  printf("> set_rtc_time\n");
#endif
  datetime_t rtc;
  int32_t epoch;
  int16_t timezone;

  /* Message will look like: 
     * c8|e6605481db318236|38700f84|ffff\r\n
     * ^         ^             ^       ^
     * 200 hardware key    epoch(UTC) timezone (here -1) 
     * 
     * timezone is currently hours only - review
     * 
     */

  epoch = hex2int32(data->incomingdata.timemessage.timestamp);

  // Pico rtc isn't timezone aware so we need to correct the UTC
  // 3600 = seconds in an hour

  timezone = hex2int16(data->incomingdata.timemessage.timezone);
  epoch += (timezone * 3600);

  epoch_to_rtc(epoch, &rtc);

  rtc_set_datetime(&rtc);

  stationdata.timezone = timezone;

#ifdef DEBUG
  printf("...Time set\n");
  printf("...RTC is: %i-%i-%i %02i:%02i:%02i Timezone: %s\n",
         rtc.year,
         rtc.month,
         rtc.day,
         rtc.hour,
         rtc.min,
         rtc.sec,
         stationdata.timezone);
#endif

#ifdef TRACE
  printf("< set_rtc_time\n");
#endif
}

void set_station_data(char *buffer, stationData *stationdata)
{
#ifdef TRACE
  printf("> set_station_data\n");
#endif
  //201;e6605481db318236;Curbar AWS;53.27044,-1.62351;180;

  char *name, *position, *altitude;

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

void report_station_details(void)
{
#ifdef TRACE
  printf("< sreport_station_details\n");
#endif

#ifdef TRACE
  printf("< report_station_details\n");
#endif
}

void open_uart(void)
{

#ifdef TRACE
  printf("> open_uart\n");
#endif

  RX_buffer_pointer = RX_buffer; // inittialise the buffer
                                 // updated on interrupt

  memset(RX_buffer, '\0', RX_BUFFER_SIZE);

  uart_init(UART_ID, BAUD_RATE);
  uart_set_translate_crlf(UART_ID, false); // don't translate cr to lf

  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Read anything left over in the modem buffer


  while (uart_is_readable(UART_ID))
  {
    char not_used = uart_getc(UART_ID);
  }

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

void on_uart_rx()
{

#ifdef TRACE_ISR
  printf("> on_uart_rx\n");
#endif
  /* Be careful with the DEBUG and TRACE below, they slow down the 
     * ISR with some odd effects
     */

  // if interrupt time is 0 - treat this as a new message
  if (UART_RX_interrupt_time == 0)
    UART_RX_interrupt_time = time_us_32();

  int chars_rx = 0;

  while (uart_is_readable(UART_ID))
  {
    *RX_buffer_pointer = uart_getc(UART_ID);
    //sleep_ms(10);
    RX_buffer_pointer++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;    }
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
