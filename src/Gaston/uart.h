#ifndef UART_H
#define UART_H


void init_uart (uint32_t baudrate);

void uart_transmit (uint8_t c);

uint8_t uart_receive (void);

void uart_sendstring (char *str);

uint8_t uart_available(void);

uint8_t check_RFID(void);


#endif
