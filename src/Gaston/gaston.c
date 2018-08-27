
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "gaston.h"
#include "adc.h"
#include "uart.h"
#include "leds.h"
#include "drive.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define WHITE_BUTTON_PRESSED !(PIND & (1<<4))
#define GREEN_BUTTON_PRESSED !(PINC & (1<<6))
#define ORDER_TIMEOUT 10



volatile uint8_t IR_ON=0;
volatile uint16_t servocnt=0;
volatile uint8_t  servopos=0;

// ISR for 16-bit-timer1 compare 1A match
// handles IR- and Servo-PWM
// TBD: improve: use HW PWMs
ISR(TIMER1_COMPA_vect) 
{
	if (IR_ON) PORTB ^= (1<<1);  // 36kHz IR pulse

	servocnt++;
	if (servocnt == 1441)  // 20ms interval 
		if (servopos) PORTF|=(1<<5);
		
    if (servocnt == 1441+servopos)	
    {
		if (servopos) PORTF &= ~(1<<5);
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


void blink_eye(uint8_t speed) {
	int i;
    // close and open lid
    for (servopos=100;servopos<130;servopos++)  for (i=0;i<speed;i++) _delay_ms(1);
    for (servopos=130;servopos>100;servopos--)  for (i=0;i<speed;i++) _delay_ms(1);
    _delay_ms(50);
    servopos=0;
}


int8_t take_order(void) {
	int seconds=0,i;
	uint8_t act_drink=0;
	uint8_t already_chosen=0;
	uint32_t timer=0;    

    while((timer < ORDER_TIMEOUT*1000) && (!cup_present())) {
        _delay_ms(20);
        timer+=20;
        if ((timer % 800) < 400) 	set_leds(LEDS_GREEN); else 	set_leds(LEDS_OFF);
    }

    if (!cup_present()) return(-1);   // order timed out !

    set_leds(LEDS_BLUE);
	play_sound('a');          // take order sound
    _delay_ms(500);

	while(cup_present()) {
        if( WHITE_BUTTON_PRESSED){ // next drink
		    set_leds(LEDS_GREEN|LEDS_RED); 
			act_drink= (act_drink+1) % AVAILABLE_DRINKS;
  	        play_sound('m'+act_drink);   // drink indicator sounds
			_delay_ms(500);
			already_chosen=1;
			set_leds(LEDS_BLUE);
  	        while (WHITE_BUTTON_PRESSED) ;  // wait for button release            
        }
        else if ((GREEN_BUTTON_PRESSED) && (already_chosen)){  // choose this drink
			set_leds(LEDS_GREEN);
  	        play_sound('b');             // accept sound
  	        _delay_ms(1000);             // wait until sound played
            return(act_drink);
        }
    }
    return(-1);   // cup was removed !
}


#define IR_TIMEOUT 100
#define IR_ONTIME  20
#define IR_OFFTIME 5

void send_IR_code (uint32_t code)
{
	uint32_t i;
	IR_ON=1;
	for (i=0;i<=32;i++) {
		if (code & ((uint32_t)1<<i)) _delay_ms(IR_ONTIME); 
		else _delay_ms(IR_OFFTIME);
		IR_ON ^= 1;
	}
	IR_ON=0;
	PORTB &= ~(1<<1);
}


void request_delivery (uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)
{
	uint32_t code=0xA5000000;
	p1 &= 0x0f; p2 &= 0x0f; p3 &= 0x0f; p4 &= 0x0f;
	code |= (((uint32_t)p4 << 20) | ((uint32_t)p3<<16) | ((uint32_t)p2<<12) | ((uint32_t)p1<<8));
	code |= ((p4 << 4) + p3) ^ ((p2<<4) | p1);
    send_IR_code(code);
}



void init_gpio()
{
    DDRB |=(1<<1);          // IR LED 
    DDRF =(1<<5);          // Servo (Eyelid)
    DDRD |=(1<<1);          // Big Greed LED 
      
    DDRD &= ~(1<<0);        // Cup Recognition Input
    PORTD |= (1<<0);        // Pullup

    DDRD &= ~(1<<4);        // Button1 (White) Input
    PORTD |= (1<<4);        // Pullup

    DDRC &= ~(1<<6);        // Button2 (Green) Input
    PORTC |= (1<<6);        // Pullup
}



// -----------------MAIN---------------/
void main () 
{
    uint8_t station=0;  //,coming_from_delivery=0;
    int8_t  selected_drink;
    uint16_t threshold;
    uint8_t delivery;    
    
    cli();
    PRR1 |= (1<<PRUSB);   // disable USB interrupts - this is necessary to use interrupts on the ATmega32u4
    CPU_PRESCALE(0);      // just to make sure we're running at full speed

    init_gpio();
    init_uart(9600); 
    init_adc();
    init_leds();
    init_motors();
    init_timer();
    set_leds(LEDS_OFF);
    sei();
    
    PORTD |= (1<<1);   // Big Led on
    stop_sound();      // stop running song (if any)
    _delay_ms(1000);
    blink_eye(10);      // welcome blink :)
    _delay_ms(700);
    play_sound('f');

    /* 
    // for testing IR transmission 
	while (1) {
        if( WHITE_BUTTON_PRESSED){ 
			set_leds(LEDS_BLUE|LEDS_RED|LEDS_GREEN);
			request_delivery (10,0,0,0);
		}
        if( GREEN_BUTTON_PRESSED){ 
			set_leds(LEDS_GREEN);
			request_delivery (0,10,0,0);
		}
		
	}
	*/

    set_leds(LEDS_RED);                // red: indicate calibration
    threshold=get_threshold();    

    while(1) {

		set_leds(LEDS_OFF);           // blue: indicate wait for cup
   
		followLine(threshold);
		station=check_RFID();
		if (station) {               // station found: ready to take an order
			stop_sound();
			stop_motors();
			blink_eye(7);
			blink_leds(station,LEDS_GREEN|LEDS_BLUE,200);

			selected_drink=take_order();
			if (selected_drink>-1) {
					
				play_drinkSong(selected_drink);   // start a song				
				get_drink(station,selected_drink,threshold);
				
				set_leds(LEDS_GREEN);     
				play_sound('e');   // voila - drink should be here !
				_delay_ms(2000);

				uint32_t timer=0;
				while (cup_present()) {    // wait for glass removal
					_delay_ms(20);
					timer+=20;
					if ((timer % 400) < 200) set_leds(LEDS_GREEN); else set_leds(LEDS_OFF);
				}    
				set_leds(LEDS_OFF);     
				_delay_ms(500);

			} else play_sound('g');

			
			if ((station==NUM_STATIONS) || ((station==2) && (get_direction()==TO_BASE))) {
				make_u_turn();
			}
		} 
    }   
 }

