#include <avr/io.h> 

#include "gaston.h"
#include "uart.h"

#ifndef F_CPU
#define F_CPU 16000000
#endif

uint8_t drink_songs[AVAILABLE_DRINKS]= { '1', '2', '3' };


void init_uart (uint32_t baudrate) 
{ 
 uint16_t ubrr = (F_CPU/8/baudrate)-1;  // Set baud rate, use double speed 
 UBRR1H = (unsigned char)(ubrr>>8); 
 UBRR1L = (unsigned char)(ubrr&0xff); 
 UCSR1A |= (1<<U2X1); // Enable receiver and transmitter 
 UCSR1B = ( 1<<TXEN1 | 1<<RXEN1);
}

void uart_transmit (uint8_t c)
{
	while((UCSR1A & (1<<UDRE1)) == 0); //wait until free to send
	UDR1 = c;
}

void uart_sendstring (char *str)
{
	while( *str != 0)
	{
		uart_transmit (*str);
		str++;
	}
}


uint8_t uart_receive (void)
{
	PORTB |= (1<<2);
	while ((UCSR1A & (1<<RXC1)) == 0); //wait until something received
	
	PORTB &= ~(1<<2);
	return (UDR1); //read received byte and return

}


uint8_t uart_available(void)
{
	if (UCSR1A & (1<<RXC1)) return (1);
    return(0);
}


void play_sound(char c)
{
	uart_transmit(c);
}

void play_drinkSong(uint8_t i)
{
	play_sound(drink_songs[i]);
}


void stop_sound()
{
	play_sound('0');
}

void printDebugMessage(char * str)
{
	uart_transmit('?');
	uart_sendstring(str);
	uart_transmit('\n');
	uart_transmit('!');	
}


