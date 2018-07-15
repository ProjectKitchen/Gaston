
#include <avr/io.h>

void init_leds()
{
    DDRB |= (1<<2);  // LEDStripe blue 
    DDRB |= (1<<6);  // LEDStripe red
    DDRB |= (1<<3);  // LEDStripe green
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
