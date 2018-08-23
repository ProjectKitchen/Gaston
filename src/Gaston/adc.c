#ifndef F_CPU 
#define F_CPU 16000000
#include <util/delay.h> 
#include <avr/io.h> 

void init_adc (void) {
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); 
	ADCSRA |= (1<<ADEN); 
}

uint16_t read_adc (uint16_t channel )
{
	ADMUX = (1<<REFS0) | channel;    // select channel MUX
    ADCSRA |= (1<<ADSC);   			 // single conversion
    while (ADCSRA & (1<<ADSC));      // wait for completion
    return ADCW;                                 
}


void int_to_str (uint16_t val, char * target)   
{
	int8_t i = 0;
	
	for (i=3;i>=0; i--)
	{
		target[i] = (val % 10) + '0';
		val /= 10;
	}
	
	target[4] = 0;

}


#endif
