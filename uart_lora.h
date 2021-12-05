#ifndef UART_LORA_H_
#define UART_LORA_H_

// function prototypes

void set_rtc_time(char * buffer, stationData * stationdata);
void set_station_data(char * buffer, stationData * stationdata);
void report_station_details();
void process_RX_message (const char * buffer);
void open_uart(void);
void on_uart_rx();

#endif
