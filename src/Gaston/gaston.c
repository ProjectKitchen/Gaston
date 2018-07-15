
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "adc.h"
#include "uart.h"
#include "leds.h"
#include "drive.h"


void take_order(void) {

    while(1){

        if(!(PIND &(1<<0))){ // white button
            break;  //TBD: drink selection
        }
        if (!(PIND&(1<<4))){  // green button
            turn();
            break;  // turn and get drink
        }
       
    }
    //~ PORTB &= ~(1<<6);
}


// -----------------MAIN---------------/
void main () 
{
    uint8_t station=0;
    uint16_t threshold;
      
    DDRD &= ~(1<<4);        // Button1 Input
    DDRD &= ~(1<<0);        // Button2 Input
    PORTD |= (1<<4);        // Button1 Pullup
    PORTD |= (1<<0);        // Button2 Pullup

    init_uart(9600); 
    init_adc();
    init_leds();
    init_motors();
    
    set_leds(2);
    threshold=get_threshold();    
    uart_transmit('1');

    while(1) {
   
        followLine(threshold);
        station=check_RFID();
        if (station) {
            uart_transmit('2');  //take order
            stop();
            take_order();
        }
    }
   
 }
