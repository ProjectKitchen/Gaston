
#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "leds.h"

void init_leds()
{
    DDRB |= (1<<2);  // LEDStripe red 
    DDRB |= (1<<6);  // LEDStripe green
    DDRB |= (1<<3);  // LEDStripe blue
}


void set_leds(uint8_t col)
{
    PORTB &= ~(1<<2);
    PORTB &= ~(1<<6);
    PORTB &= ~(1<<3);
    
    if (col & 1) PORTB |= (1<<2);
    if (col & 2) PORTB |= (1<<6);
    if (col & 4) PORTB |= (1<<3);
}

void blink_leds(uint8_t count, uint8_t color, uint16_t delay)
{
    uint8_t i,t;
    for (i=0;i<count;i++) {
		set_leds(color);
		for (t=0;t<delay;t++) _delay_ms(1);
		set_leds(LEDS_OFF);
		for (t=0;t<delay;t++) _delay_ms(1);
		
	}
}

void blink_sos()
{
	uint8_t i;
	for(i=0;i<3;i++) {
		set_leds(LEDS_RED);
		_delay_ms(100);
		set_leds(LEDS_OFF);
		_delay_ms(100);
	}
	for(i=0;i<3;i++) {
		set_leds(LEDS_RED);
		_delay_ms(200);
		set_leds(LEDS_OFF);
		_delay_ms(200);
	}
	for(i=0;i<3;i++) {
		set_leds(LEDS_RED);
		_delay_ms(100);
		set_leds(LEDS_OFF);
		_delay_ms(100);
	}
}
	
	
 
