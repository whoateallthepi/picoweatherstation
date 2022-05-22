#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <string.h>
#include "constants.h"

#include "types.h"
#include "rak811_lorawan.h"
#include "utilities.h"
#include "downlink.h"

//#define DEBUG
//#define TRACE
extern stationData stationdata;
// Modem commands
const char *VERSION = "at+version\r\n";
const char *SET_LORAWAN = "at+set_config=lora:work_mode:0\r\n";
const char *SET_OTAA = "at+set_config=lora:join_mode:0\r\n";
const char *CLASS_A = "at+set_config=lora:class:0\r\n";
const char *SET_EU868 = "at+set_config=lora:region:EU868\r\n";
const char *SET_DEVICE_EUI = "at+set_config=lora:dev_eui:%s\r\n";
const char *SET_APP_EUI = "at+set_config=lora:app_eui:%s";
const char *SET_APP_KEY = "at+set_config=lora:app_key:%s";
const char *RESTART = "at+set_config=device:restart\r\n";
const char *SEND_LORAWAN = "at+send=lora:2:%s%s";
const char *JOIN = "at+join\r\n";

// Modem reponses
const char *JOIN_SUCCESS = "OK Join Success\r\n";
const char *INCOMING_DATA = "OK \r\nat+recv=";
const char *MODEM_ERROR = "ERROR: ";
const char *MODEM_OK = "OK \r\n";

const char *RAK811_RECVD = "at+recv="; // This is the start of a data message

char RAK811_DATA_END[] = {0x0D, 0x0A, 0x00}; // This is how a RAK811
                                              // incoming data message ends

extern int sendstationreport; // Set to 1 to force a downlink of station data

int rak811_lorawan_initialise(void)
{
  char command_buffer[RAK811_COMMAND_BUFFER_SIZE];
  char response_buffer[RAK811_COMMAND_BUFFER_SIZE];

  // open the UART

  uart_init(UART_ID, BAUD_RATE);
  uart_set_translate_crlf(UART_ID, false); // don't translate cr to lf

  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_set_hw_flow(UART_ID, false, false);
  uart_set_fifo_enabled(UART_ID, true);

  // The first few UART responses often have 'garbage' characters (possiply from the Pico) so doing
  // a version call here and ignoring the result usually syncs things up.

  rak811_command(VERSION, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  /* This is the initialise sequence - but retaned by the modem so do offline instead

  RAK811_command(SET_LORAWAN, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  RAK811_command(SET_OTAA, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  RAK811_command(CLASS_A, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  RAK811_command(SET_EU868, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  snprintf(command_buffer, RAK811_COMMAND_BUFFER_SIZE, SET_DEVICE_EUI, LORAWAN_DEVICE_EUI);
  RAK811_command(command_buffer, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  snprintf(command_buffer, RAK811_COMMAND_BUFFER_SIZE, SET_APP_EUI, LORAWAN_APP_EUI);
  RAK811_command(command_buffer, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  snprintf(command_buffer, RAK811_COMMAND_BUFFER_SIZE, SET_APP_KEY, LORAWAN_APP_KEY);
  RAK811_command(command_buffer, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_COMMAND_WAIT_MS);

  // Do some error checking
  */

  return 0;
}

int rak811_lorawan_join(void)
{
  char command_buffer[RAK811_COMMAND_BUFFER_SIZE];
  char response_buffer[RAK811_COMMAND_BUFFER_SIZE];
  int join_response = 0;

  // At the moment ignoring responses from below

  int counter = 0;
  do
  {
    join_response = rak811_command(JOIN, response_buffer, RAK811_COMMAND_BUFFER_SIZE, RAK811_JOIN_WAIT_MS);
    counter++;
  } while ((counter < RAK811_JOIN_ATTEMPTS) && (join_response != 0));

  if (join_response != 0) {
      stationdata.network_status = NETWORK_DOWN;
      led_double_flash(TX_LED);
    }
    else
    {
      stationdata.network_status = NETWORK_UP;
      led_flash3_leds(BOOT_LED, TX_LED, RX_LED);
    }

  return join_response; // Zero = success
}

int rak811_command(const char *command, char *response, int response_size, int wait_ms)
{
  /* Puts a command to the RAK811 modem
   * returns 0 if the response starts OK
   * returns error code nn if the response is ERROR nn
   * otherwise returns -1.
   * The text of the response is in response - for further processing if required
   * wait_ms is how many milliseconds we wait for the response
   * most commands are quick but at+join, for example, can take seconds
   */
  char command_response[RX_BUFFER_SIZE];
  char *command_response_ptr;
  uint8_t chars_rx = 0;

  memset(command_response,0, RX_BUFFER_SIZE); //to help me debug only

  // clear the FIFO and ignore the data 

  while (uart_is_readable(UART_ID))
  {
    uart_getc(UART_ID);
    busy_wait_us(100); // slow me down!
  }

  uart_puts(UART_ID, command);
  //busy_wait_ms(wait_ms);
  char x;
  command_response_ptr = command_response;
  
  // Will always get omething from modem - hopefully OK, but maybe an error

  do 
  {
    *command_response_ptr = uart_getc(UART_ID);
    busy_wait_us(200); // slow me down!
    command_response_ptr++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  } while (uart_is_readable(UART_ID));

  // Keep checking the UART for a few seconds - in case there is downlonk data to follow 

  if (uart_is_readable_within_us(UART_ID, (1000 * RAK811_DOWNLINK_WAIT)))
  {
    do
    { // Read the downlink into same buffer if it is there
      *command_response_ptr = uart_getc(UART_ID);
      busy_wait_us(200); // slow me down!
      command_response_ptr++;
      chars_rx++;
      if (chars_rx >= RX_BUFFER_SIZE)
      {
        printf("...ERROR: Input buffer overload\n");
        assert(chars_rx >= RX_BUFFER_SIZE);
        break;
      }
    } while (uart_is_readable(UART_ID));
  }  

  *(command_response_ptr) = '\0'; // terminate the string

  command_response_ptr = command_response;

  if (strncmp(command_response, "OK", 2) == 0)
  {
    /* So far so good, we are working as a Class A lorawan device, so we may get and OK from the modem, 
     * plus an incoming message eg "OK \r\nat+recv=0,-18,5,0\r\n" o check for this
    */
    if (strlen(command_response) < 6) 
    { // This is just an 'OK \r\n'
      strncpy(response, command_response, response_size);
      return 0;
    }
    // Check for success response from at+join which is 'OK Join Success\r\n'

    if (strncmp(command_response, JOIN_SUCCESS, strlen(JOIN_SUCCESS)) == 0)
    {
      strncpy(response, command_response, response_size);
      return 0;
    }

    // This looks like an incoming message - check "OK \r\nat+recv=0,-18,5,0

    if ( strncmp(command_response, INCOMING_DATA, strlen(INCOMING_DATA)) != 0)
    {  
      // Something odd here
      strncpy(response, command_response, response_size);
      return -2;    
    }

    // OK we have data - deal with it here (consider returning a response and dealing in main process?)
    rak811_lorawan_process_downlink(command_response+strlen(MODEM_OK));
    return 0;
  }
  // Now almost certainly in an error state try and get the error number and return that...

  if (strncmp(command_response, MODEM_ERROR, strlen(MODEM_ERROR)) == 0)
  {
    strncpy(response, command_response, response_size); // copy error message - should be ERROR nn so 8 char max
     
    char *not_used;
    int error_num = strtol(command_response+strlen(MODEM_ERROR), &not_used, 10); 
    if (error_num == 0)
      error_num = -1;
    return error_num; // hopefully this is the error number!
  }
  
  // Not sure how we got here - return -3
  strncpy(response, command_response, response_size);
  return -3;
}


int rak811_read_response(char *data, uint data_size)
{
  /* Utility to read response form the RAK811 modem
   *
   * If response is OK - returns 0, If ERROR returns error code, otherwise
   * returns -1 and the text in data as a null-terminated string.
   */

  char response[RX_BUFFER_SIZE];
  char *response_ptr;
  uint8_t chars_rx = 0;

  response_ptr = response;

  while (uart_is_readable(UART_ID))
  {
    *response_ptr = uart_getc(UART_ID);
    // sleep_ms(10);
    *response_ptr++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  }

  *(response_ptr + 1) = 0; // terminate the string

  if (strncmp(response, "OK", 2) == 0)
  {
    strncpy(data, response, data_size); // at+join, returns OK Join Success
    return 0;

    if (strncmp(response, "ERROR", 5) == 0)
    {
      strncpy(data, response, 8); // copy error message - should be ERROR nn so 8 char max
      return atoi(data + 7);      // hopefully this is the error number!
    }
    // Not sure how we got here - return -1
    strncpy(data, response, data_size);
    return -1;
  }
}

void rak811_lorawan_put_hex(char *data, int length)
{

  char tx_buffer[TX_BUFFER_SIZE];
  char data_hex[1000]; // each ascii char will take 2 hex
  char response[RX_BUFFER_SIZE];
  char *response_ptr;
  uint8_t chars_rx = 0;
  int command_response = 0;

  // check network status - if we haven't joined / error tate - try rejoining

  if (stationdata.network_status != NETWORK_UP) rak811_lorawan_join();

  snprintf(tx_buffer, TX_BUFFER_SIZE, SEND_LORAWAN, data, "\r\n");

  led_flash(TX_LED);
  
  // Will try uplink anyway - without testing network status. Maybe add a further check here?
  command_response = rak811_command(tx_buffer, response, RX_BUFFER_SIZE, RAK811_SEND_WAIT);
  if ( command_response != 0) led_double_flash(TX_LED);
}

void rak811_lorawan_process_downlink(char *message)
{
  incomingMessage im; // will contain all the data from the message
  int chars_received = 0;
  int32_t rssi = 0; // Signal strength and signal/noise ratio from modem
  int32_t snr = 0;
  int32_t lora_port = 0;

  int type_i; // will hold mesage type as an int

  int x; // used to scan the buffer

  chars_received = rak811_lorawan_parse_incoming(message, &im,  &lora_port, &rssi, &snr);

  if (chars_received != 0) 
  {  
    type_i = hex2int(im.messagetype);

    switch (type_i)
    {
    case 200:
      set_timezone(&im);
      stationdata.send_station_report = 1; // Send a 101 message  to confirm at next opportunity
      break;
    case 201:
      set_station_data(&im);
      stationdata.send_station_report = 1; // Send a 101 message  to confirm at next opportunity
      break;
    case 202:
      stationdata.send_station_report = 1; // Send a 101 message  to confirm at next opportunity
    case 203:
      // Reboot
      software_reset();
      

    default:
      led_flash2_leds (TX_LED, RX_LED); // Flash an error 
    }
  }
}

int rak811_lorawan_parse_incoming(char *data, incomingMessage *im, int32_t *port, int32_t *RSSI, int32_t *SNR)
{
  /* Data received from the RAK811 in lorwan mode is as follows:
     * at+recv=1,-69,6,113:3130303B.....
     *          
     *     port,RSSI, SNR,CHARS:  the data as a hex encoded string
     * 
     * To process, strip off the head, store the RSSI (signal strength)
     * and SNR (signal to noise ratio) and do some work on the message
     * 
    */

  char *rssi;
  char *snr;
  char *char_count;
  char *lora_port;
  char *hex_data;

  int char_count_int = 0;

  if ((strncmp(RAK811_RECVD, data, RAK811_RECVD_HEADER_LEN) == 0))
  {
    // This is a valid data received message - copy it
    //    strncpy(data_copy,(data + RAK811_RECVD_HEADER_LEN),RX_BUFFER_SIZE); // removes at+recv
  }
  else
  {
    printf("ERROR - invalid RAK811 message: %s\n", data);
    return 0;
  }

  lora_port = strtok(data + RAK811_RECVD_HEADER_LEN, ",");
  *port = atoi(lora_port);
  rssi = strtok(NULL, ",");
  *RSSI = atoi(rssi);
  snr = strtok(NULL, ",");
  *SNR = atoi(snr);
  char_count = strtok(NULL, ":");
  char_count_int = atoi(char_count);
  
  if (char_count_int == 0) return 0; // This is probably just an 'ACK' from the network - ignore

  hex_data = strtok(NULL, RAK811_DATA_END); // messages are terminated by CR NL

  strncpy((char *)im, hex_data, sizeof(incomingMessage));

  return char_count_int;
}