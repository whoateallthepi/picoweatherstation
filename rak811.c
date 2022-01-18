#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <string.h>
#include "constants.h"

#include "types.h"
#include "rak811.h"
#include "utilities.h"

#define LORA_RECEIVE 1
#define LORA_SEND 2

//#define DEBUG
//#define TRACE

const char *SET_RECEIVE = "at+set_config=lorap2p:transfer_mode:1\r\n";
const char *SET_SEND = "at+set_config=lorap2p:transfer_mode:2\r\n";

const char *SEND_P2P = "at+send=lorap2p:%s\r\n";

const char *REQUEST_RESET = "at+set_config=device:restart\r\n";

const char *RAK811_RECVD = "at+recv="; // This is the start of a data message

const char RAK811_DATA_END[] = {0x0D, 0x0A, 0x00}; // This is how a RAK811
                                                   // incoming data message ends
void rak811_set_mode(uint8_t mode)
{
#ifdef TRACE
  printf("> rak811_set_mode. Mode: %u\n", mode);
#endif
  //
  uint8_t rx_buffer[RX_BUFFER_SIZE];
  char *rx_buffer_pointer;
  int chars_rx = 0;

  // Switch off uart interrupts - we are communicating with the
  // modem

  uart_set_irq_enables(UART_ID, false, false);

  if (mode == LORA_SEND)
  {
#ifdef DEBUG
    printf("...setting LORA mode 2 (send)\n");
#endif
    uart_puts(UART_ID, SET_SEND);
  }
  else if (mode == LORA_RECEIVE)
  {
#ifdef DEBUG
    printf("...setting LORA mode 1 (receive)\n");
#endif

    uart_puts(UART_ID, SET_RECEIVE);
  }

  while (!uart_is_readable(UART_ID))
  {
    busy_wait_us(1000);
  }

  busy_wait_us(RAK811_WAIT);

  // read back the OK from the modem - this is just discarded
  rx_buffer_pointer = rx_buffer;

  while (uart_is_readable(UART_ID))
  {
    *rx_buffer_pointer = uart_getc(UART_ID);
    //sleep_ms(10);
    rx_buffer_pointer++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  }

  *(rx_buffer_pointer + 1) = 0; //terminate the string

#ifdef DEBUG
  printf("... response from RAK811: %s\n", rx_buffer);
#endif

  /* If we have set the modem to 'receive' we need to restore
     * UART interrupts to deal with incoming messages. If we are 
     * going on to send data, leave interrupts off and these will 
     * be restored when rak811_set_mode is called again to set to 
     * receive
     */

  // re-enable the UART to send interrupts - RX only
  if (mode == LORA_RECEIVE)
    uart_set_irq_enables(UART_ID, true, false);

#ifdef TRACE
  printf("< rak811_set_mode\n");
#endif

  return;
}

void rak811_puts(char *data)
{
#ifdef TRACE
  printf("> rak811_puts\n");
#endif
  char tx_buffer[TX_BUFFER_SIZE];
  char data_hex[1000]; // each ascii char will take 2 hex
  char response[RX_BUFFER_SIZE];
  char *response_ptr;
  uint8_t chars_rx = 0;

  rak811_set_mode(LORA_SEND);

  string2hexString(data, data_hex);

  sprintf(tx_buffer, SEND_P2P, data_hex, "\r\n");

#ifdef DEBUG
  printf("...about to send: %s\n", tx_buffer);
#endif

  led_on(TX_LED);
  uart_puts(UART_ID, tx_buffer);
  led_off(TX_LED);

#ifdef DEBUG
  printf("...about to wait");
#endif

  busy_wait_us(RAK811_SEND_WAIT);

#ifdef DEBUG
  printf("   ...waaaiiittt over\n");
#endif

  // read back the OK from the modem - this is just discarded

  response_ptr = response;

  while (uart_is_readable(UART_ID))
  {
    *response_ptr = uart_getc(UART_ID);
    //sleep_ms(10);
    *response_ptr++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  }

  *(response_ptr + 1) = 0; //terminate the string

  if (0)
  {
    rak811_reset();
  }

#ifdef DEBUG
  printf("... response from RAK811: %s\n", response);
#endif

  /* There are problems with the modem getting locked in some
     * state that leads to repeated ERROR 5 errors. The exact cause
     * remains undiagnosed so, for now, reset the modem and try to resend
     */

  /*    if (strncmp(response,"ERROR", 5) == 0) {
        #ifdef DEBUG
          printf("... error writing to RAK811... resetting\n");
        #endif
        rak811_reset();
        // try it again (tidy up this code please)
        
        #ifdef DEBUG
          printf("... trying to resend message\n");
        #endif
        
        chars_rx = 0;
        response_ptr = response;
        uart_puts(UART_ID, tx_buffer);  
        sleep_ms(RAK811_WAIT); 
        // read back the OK (?) from the modem  
        while (uart_is_readable(UART_ID)) {
            *response_ptr = uart_getc(UART_ID);
            //sleep_ms(10);
            *response_ptr++;
            chars_rx++;
            if (chars_rx >= RX_BUFFER_SIZE){
                printf("...ERROR: Input buffer overload\n");
                assert(chars_rx >= RX_BUFFER_SIZE);
                break;
            }
        }
        #ifdef DEBUG
          printf("... response from RAK811 (2nd try): %s\n", response);
        #endif
     } */

  // set back to receive

  rak811_set_mode(LORA_RECEIVE);

#ifdef TRACE
  printf("< rak811_gets\n");
#endif
}

void rak811_put_hex(char *data, int length)
{
#ifdef TRACE
  printf("> rak811_put_hex\n");
#endif
  char tx_buffer[TX_BUFFER_SIZE];
  char data_hex[1000]; // each ascii char will take 2 hex
  char response[RX_BUFFER_SIZE];
  char *response_ptr;
  uint8_t chars_rx = 0;

  rak811_set_mode(LORA_SEND);

  //data2hexString(data, data_hex, length);

  //string2hexString(data, data_hex);

  sprintf(tx_buffer, SEND_P2P, data, "\r\n");

  printf("...about to send:\n");

#ifdef DEBUG
  printf("...about to send: %s\n", tx_buffer);
#endif

  led_on(TX_LED);
  uart_puts(UART_ID, tx_buffer);
  led_off(TX_LED);

#ifdef DEBUG
  printf("...about to wait");
#endif

  busy_wait_us(RAK811_SEND_WAIT);

#ifdef DEBUG
  printf("   ...waaaiiittt over\n");
#endif

  // read back the OK from the modem - this is just discarded

  response_ptr = response;

  while (uart_is_readable(UART_ID))
  {
    *response_ptr = uart_getc(UART_ID);
    //sleep_ms(10);
    *response_ptr++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  }

  *(response_ptr + 1) = 0; //terminate the string

  if (0)
  {
    rak811_reset();
  }

#ifdef DEBUG
  printf("... response from RAK811: %s\n", response);
#endif

  rak811_set_mode(LORA_RECEIVE);

#ifdef TRACE
  printf("< rak811_gets\n");
#endif
}

void rak811_reset()
{
#ifdef TRACE
  printf("> rak811_reset\n");
#endif

  char response[RX_BUFFER_SIZE];
  char *response_ptr;
  uint8_t chars_rx = 0;

  uart_puts(UART_ID, REQUEST_RESET);

  busy_wait_us(RAK811_WAIT);

  // read back the OK (hopefully) from the modem

  response_ptr = response;

  while (uart_is_readable(UART_ID))
  {
    *response_ptr = uart_getc(UART_ID);
    //sleep_ms(10);
    response_ptr++;
    chars_rx++;
    if (chars_rx >= RX_BUFFER_SIZE)
    {
      printf("...ERROR: Input buffer overload\n");
      assert(chars_rx >= RX_BUFFER_SIZE);
      break;
    }
  }

  *(response_ptr + 1) = 0; //terminate the string

#ifdef DEBUG
  printf("... response from RAK811: %s\n", response);
#endif

#ifdef TRACE
  printf("< rak811_reset\n");
#endif
}

int rak811_process_incoming(char *data, incomingMessage *im, int32_t *RSSI, int32_t *SNR)
{
  /* Data received from the RAK811 in p2p mode is as follows:
     * at+recv=-69,6,113:3130303B.....
     *     ^    ^  ^  ^       ^
     * head  RSSI SNR CHARS   the data as a hex encoded string
     * 
     * To process, strip off the head, store the RSSI (signal strength)
     * and SNR (signal to noise ratio) and do some work on the message
     * 
    */

  char *rssi;
  char *snr;
  char *char_count;
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

  rssi = strtok(data + RAK811_RECVD_HEADER_LEN, ",");
  *RSSI = atoi(rssi);
  snr = strtok(NULL, ",");
  *SNR = atoi(snr);
  char_count = strtok(NULL, ":");
  char_count_int = atoi(char_count);
  hex_data = strtok(NULL, RAK811_DATA_END); // messages are terminated by CR NL

#ifdef DEBUG
  printf("... rssi: %s\n    snr: %s\n    count: %s\n    hex: %s\n",
         rssi, snr, char_count, hex_data);
#endif

  strncpy((char *)im, hex_data, sizeof(incomingMessage));

  return char_count_int;
}

void rak811_format_recvd(char *data)
{

  /* Data received from the RAK811 in p2p mode is as follows:
     * at+recv=-69,6,113:3130303B.....
     *     ^    ^  ^  ^       ^
     * head  RSSI SNR CHARS   the data as a hex encoded string
     * 
     * To process, strip off the head, store the RSSI (signal strength)
     * and SNR (signal to noise ratio) and bash out the rest as 
     * a C string.
     * 
     * This version writes back to the original input string
     * 
     */
  char data_copy[RX_BUFFER_SIZE];
  char *rssi;
  char *snr;
  char *char_count;
  char *hex_data;

#ifdef TRACE
  printf("> rak811_format_recvd\n");
#endif

  if ((strncmp(RAK811_RECVD, data, RAK811_RECVD_HEADER_LEN) == 0))
  {
    // This is a valid data received message - copy it
    strcpy(data_copy, (data + RAK811_RECVD_HEADER_LEN)); // removes at+recv
  }
  else
  {
    printf("ERROR - invalid RAK811 message: %s\n", data);
    return;
  }

  rssi = strtok(data_copy, ",");
  snr = strtok(NULL, ",");
  char_count = strtok(NULL, ":");
  hex_data = strtok(NULL, RAK811_DATA_END); // messages are terminated by CR NL

#ifdef DEBUG
  printf("... rssi: %s\n    snr: %s\n    count: %s\n    hex: %s\n",
         rssi, snr, char_count, hex_data);
#endif

  /* Do some prescreening to filter out messages from other networks
     * If the message doesn't start with 3 digits, stop decoding and 
     * leave the input data as is. The error will be picked up later. 
     */

  if ((hex_data[0] != '3') ||
      (hex_data[2] != '3') ||
      (hex_data[4] != '3'))
  {
    printf("... input message not recognised\n");
    return;
  }

  hextobin(hex_data, data, atoi(char_count));

#ifdef DEBUG
  printf("...data decoded: %s\n", data);
#endif

#ifdef TRACE
  printf("< rak811_format_recvd\n");
#endif
}

void data2hexString(char *data, char *output, int length)
{
  // User beware - no string overrun checks
  int x;

  for (x = 0; x < (length); x++)
    //itoa( (int) data[x],output+(x*2),16);

    sprintf(output + (x * 2), "%02x", data[x]);

  printf("%u", x);
  *(output + ((x * 2) + 1)) = 0; //terminate the string

  printf("> data2hexString\n");
  //  strcpy(output, "ABCDEF0A");
  return;
}

//function to convert ascii char[] to hex-string (char[])
void string2hexString(char *input, char *output)
{

#ifdef TRACE
  printf("> string2hexString\n");
#endif

  int loop;
  int i;

  i = 0;
  loop = 0;

  while (input[loop] != '\0')
  {

    itoa(input[loop], (output + i), 16);

    //printf("loop: %d  \n", loop);

    // sprintf((char*)(output+i),"%02X", input[loop]);
    loop += 1;
    i += 2;
  }
  //insert NULL at the end of the output string
  //output[i++] = '\0';

  //#ifdef DEBUG
  //  printf("...hex string is: %s\n", output);
  //#endif

#ifdef TRACE
  printf("< string2hexString\n");
#endif
}

// As the RAK811 returns data as hex char array 313030.. etc
// The following is needed to convert to bytes

void hextobin(const char *str, uint8_t *bytes, size_t bytes_len)
{
  uint8_t pos;
  uint8_t idx0;
  uint8_t idx1;

  // mapping of ASCII characters to hex values
  const uint8_t hashmap[] =
      {
          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
          0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
          0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
      };
#ifdef TRACE
  printf("> hextobin\n");
#endif

  memset(bytes, 0, bytes_len);

  for (pos = 0; ((pos < (bytes_len * 2)) && (pos < strlen(str))); pos += 2)
  {
    idx0 = ((uint8_t)str[pos + 0] & 0x1F) ^ 0x10;
    idx1 = ((uint8_t)str[pos + 1] & 0x1F) ^ 0x10;
    bytes[pos / 2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
  };
#ifdef TRACE
  printf("< hextobin\n");
#endif
}
