
#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"

int8_t direction=1;


void init_motors()
{
    DDRB |= (1<<4);         // left drive 
    DDRB |= (1<<5);         // left direction
    DDRE |= (1<<6);         // right drive
    DDRD |= (1<<7);         // right direction
}


void turn () 
{
    PORTB |= (1<<4);
    _delay_ms(500);
    direction=-direction;  
}

void followLine (uint16_t threshold) 
{
    static uint8_t wheeltimer=0;
    uint16_t IR_sense;

    wheeltimer++;
    wheeltimer=wheeltimer%10;
    
    if (wheeltimer>=5){
    if (direction == 1) {        // forward
        IR_sense = read_adc(4);
       
        if (IR_sense > threshold) {
            PORTB |= (1<<4);
            PORTE &= ~(1<<6);
        }
        else {
            PORTB &= ~(1<<4);
            PORTE |= (1<<6);
            }
       }
   

    else if (direction == -1) {  // backwards
    
    IR_sense = read_adc(4);
       
        if (IR_sense < threshold) {
            PORTB |= (1<<4);         
            PORTE &= ~(1<<6);
        }
        else {
            PORTB &= ~(1<<4);
            PORTE |= (1<<6);
            }
            
        }
	}
	else{
		PORTB &= ~(1<<4);
		PORTE &= ~(1<<6);
	}
   
}

void stop () 
{
	//~ PORTB &= ~(1<<2);
	//~ PORTB &= ~(1<<3);
    //~ PORTB |= (1<<6);
    PORTB &= ~(1<<4);
    PORTE &= ~(1<<6);
    PORTB &= ~(1<<5);
    PORTD &= ~(1<<7);
   
}


uint16_t get_threshold()
{
    uint16_t threshold_low,threshold_high;

    while(PIND &(1<<0)!=0);
    threshold_high=read_adc(4);
    
    while(PIND &(1<<4)!=0);
    threshold_low=read_adc(4);
    
    return((threshold_low+threshold_high)/2);

}
