#ifndef RAK811_LORAWAN_H_
#define RAK811_LORAWAN_H_

#define RAK811_COMMAND_WAIT_MS 0 // 1s = 1000
#define RAK811_JOIN_WAIT_MS 20000  // 1s = 1000

#define RAK811_SEND_WAIT 10000 // Time to wait after sending data
                                 // for an 'OK' from the modem 1s = 1000


#define RAK811_DOWNLINK_WAIT 5000 // 1000 = 1s 

#define RAK811_JOIN_ATTEMPTS 5 // How many times we try and join LORAWAN network

#define RAK811_RECVD_HEADER_LEN 8

#define RAK811_COMMAND_BUFFER_SIZE 60

#define SLEEP 1 // Values for modem power control
#define WAKE 0 

// prototypes

int rak811_lorawan_initialise(void);
int rak811_lorawan_join(void);
int rak811_change_state(int state);
int rak811_command(const char *command, char *response, int response_size, int wait_ms);
int rak811_read_response(char *data, uint data_size);
void rak811_lorawan_put_hex(char *data, int length, int port);
void rak811_lorawan_process_downlink(char *message);
int rak811_lorawan_parse_incoming(char *data, incomingMessage *im, int32_t *port, int32_t *RSSI, int32_t *SNR);

#endif
