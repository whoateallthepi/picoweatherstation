#ifndef UART_LORA_H_
#define UART_LORA_H_

// function prototypes

void set_rtc_time(incomingMessage *data, uint32_t time_received);
void set_station_data(incomingMessage *data);
void report_station_details();
void process_RX_message(char *buffer, uint32_t time_received);
void open_uart(void);
void on_uart_rx();

#endif
