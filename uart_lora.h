#ifndef UART_LORA_H_
#define UART_LORA_H_

// function prototypes

void set_rtc_time(incomingMessage * data);
void set_station_data(char * buffer, stationData * stationdata);
void report_station_details();
void process_RX_message (char * buffer);
void open_uart(void);
void on_uart_rx();

#endif
