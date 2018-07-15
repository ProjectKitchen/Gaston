#include <avr/io.h> 

#ifndef F_CPU
#define F_CPU 16000000
#endif

#define NUM_CODES 3
static char codes[NUM_CODES][13]={"4500B8D1FBD7","82003CE0530D","82003CA1D6C9"};


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


uint8_t check_RFID(void) {

    static char actcode[13];
    static uint8_t actpos=0;
    static int8_t last_code=-1;
    static uint8_t rfidstate=0;
    uint8_t val,i;

    if (uart_available()) {
      
        val=uart_receive();

        switch (rfidstate) {
          case 0:  if (val==2) {
                    actpos=0;
                    rfidstate=1;
                   }
                   break;
                   
          case 1:   if ((val!=3) && (actpos<12)) {
                        actcode[actpos]=val;
                        actpos++;
                    }
                    else if (actpos==12) {
                        actcode[actpos]=0;
                        for (i=0;i<NUM_CODES;i++) {
                            if (!strcmp(actcode,codes[i]) && (i!=last_code)) {
                                last_code=i;
                                rfidstate=0;
                                return(i+1);
                            }
                        }
                        rfidstate=0;
                   }
                   else rfidstate=0;
                   break;
        }
    }
    return(0);
}


