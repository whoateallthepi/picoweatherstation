#ifndef RAK811_H_
#define RAK811_H_

#define RAK811_WAIT 500000 // Microseconds to wait for a modem response

#define RAK811_SEND_WAIT 8000000 // Microseconds after sending data 
                                 // to wait for an 'OK' from the modem 



#define RAK811 1 // Adds extra processing call for messages 
#define RAK811_RX_DELAY 1000000 // Microseconds (1s)

#define RAK811_RECVD_HEADER_LEN 8


// prototypes

void rak811_set_mode (uint8_t mode);
void rak811_puts (char * data);
void rak811_format_recvd (char * data);
void rak811_put_hex (char * data, int length);
void data2hexString(char * data, char * output, int length);
void string2hexString(char * input, char * output);
void hextobin(const char * str, uint8_t * bytes, size_t bytes_len);
void rak811_reset();

int rak811_process_incoming (char * data, incomingMessage * im, int32_t * RSSI, int32_t * SNR);

#endif
