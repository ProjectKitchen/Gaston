
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "adc.h"
#include "uart.h"
#include "leds.h"
#include "drive.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

volatile uint8_t IR_ON=0;
volatile uint16_t servocnt=0;
volatile uint8_t  servopos=35;

// ISR for 16-bit-timer1 compare 1A match
// handles IR- and Servo-PWM
// TBD: improve: use HW PWMs
ISR(TIMER1_COMPA_vect) 
{
	if (IR_ON) PORTF ^= (1<<5);  // 36kHz IR pulse

	servocnt++;
	if (servocnt == 1441)  // 20ms interval 
		if (servopos) PORTF|=(1<<6);
		
    if (servocnt == 1441+servopos)	
    {
		if (servopos) PORTF &= ~(1<<6);
		servocnt=servopos;
	}

}

void init_timer()
{	
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS10);   // 16 MHz count frequency
	TCCR1C = 0; 

	OCR1AH = 0;
	OCR1AL = 222;  // 72 kHz interrupt frequency for compare match
	
	TIMSK1 = 1 << OCIE1A;
}



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
}


// -----------------MAIN---------------/
void main () 
{
    uint8_t station=0;
    uint16_t threshold;
    
    cli();
      
    PRR1 |= (1<<PRUSB);  
    
    CPU_PRESCALE(0);

    DDRF |=(1<<5);          // IR LED 
    DDRF |=(1<<6);          // Servo (Eyelid)
      
    DDRD &= ~(1<<4);        // Button1 Input
    DDRD &= ~(1<<0);        // Button2 Input
    PORTD |= (1<<4);        // Button1 Pullup
    PORTD |= (1<<0);        // Button2 Pullup

         
    init_uart(9600); 
    init_adc();
    init_leds();
    init_motors();
    init_timer();
    
    sei();
    
    // close and open lid
    for (servopos=35;servopos<150;servopos++)  _delay_ms(20);
    for (servopos=150;servopos>35;servopos--)  _delay_ms(20);

    IR_ON=1;
    
    set_leds(2);
    threshold=get_threshold();    



    // start song
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
