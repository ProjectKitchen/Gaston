
#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"
#include "uart.h"
#include "drive.h"

#define DEFAULT_THRESHOLD 200
#define DEADZONE 10

#define RIGHT_FORWARD PORTD &= ~(1<<7); PORTE |= (1<<6)
#define RIGHT_BACK    PORTE &= ~(1<<6); PORTD |= (1<<7)
#define RIGHT_STOP    PORTE &= ~(1<<6); PORTD &= ~(1<<7);

#define LEFT_FORWARD  PORTB &= ~(1<<5); PORTB |= (1<<4)
#define LEFT_BACK     PORTB &= ~(1<<4); PORTB |= (1<<5)
#define LEFT_STOP     PORTB &= ~(1<<4); PORTB &= ~(1<<5);

uint8_t drink_recipes[AVAILABLE_DRINKS][4] = {  {15,0,0,0} , {0,15,0,0} , {0,0,15,0} };

char codes[NUM_STATIONS][13]={"4500B8D1FBD7","82003CE0530D","82003CA1D6C9"};
uint8_t last_station=0;
uint8_t direction=FROM_BASE;


void init_motors()
{
    DDRB |= (1<<4);         // left forward 
    DDRB |= (1<<5);         // left back
    DDRE |= (1<<6);         // right forward
    DDRD |= (1<<7);         // right back
}


void make_u_turn() 
{
	RIGHT_FORWARD;
	LEFT_BACK;
    _delay_ms(900);
    RIGHT_STOP;
    LEFT_STOP
    _delay_ms(200);
    
}

void followLine (uint16_t threshold) 
{
    static uint8_t wheeltimer=0;
    uint16_t IR_sense;

    wheeltimer++;
    
    // if (!(wheeltimer%2))
    {

		IR_sense = read_adc(4);
	   
		if (IR_sense > threshold+DEADZONE) {
			RIGHT_STOP;   // PORTE &= ~(1<<6);
			LEFT_FORWARD; // PORTB |= (1<<4);
		}
		else if (IR_sense < threshold-DEADZONE) {
			LEFT_STOP; //PORTB &= ~(1<<4);
			RIGHT_FORWARD; //PORTE |= (1<<6);
		} else  {
			LEFT_FORWARD;
			RIGHT_FORWARD;
		}
	}
/*	else {
			LEFT_STOP;
			RIGHT_STOP;
	}
	*/
}


void stop_motors () 
{
	LEFT_STOP;
	RIGHT_STOP;
}


uint16_t get_threshold()
{
    uint16_t threshold_low,threshold_high;

    while ((PIND & (1<<4)) !=0 ) ;
    threshold_high=read_adc(4);
    
//    while ((PINC & (1<<6))!=0) ;
//    threshold_low=read_adc(4);
    
    threshold_low=threshold_high+DEFAULT_THRESHOLD;
    
    return((threshold_low+threshold_high)/2);

}


uint8_t cup_present() 
{
	if ((PIND & (1<<0)) != 0) return(0);
	return(1);
}
	

uint8_t get_direction()
{
	return(direction);
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
                        for (i=0;i<NUM_STATIONS;i++) {
                            if (!strcmp(actcode,codes[i]) && (i!=last_code)) {
                                last_code=i;
                                rfidstate=0;
                                if (i>last_station) direction=FROM_BASE;
                                else direction=TO_BASE;
                                last_station=i;
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



void get_drink(uint8_t station, uint8_t drink, uint16_t threshold) 
{
	int i;
	for (i=0;i<10;i++) {
		set_leds(i);
		_delay_ms(200);
	}
	
	if (get_direction()==FROM_BASE) make_u_turn();
	
	while (check_RFID() != 1)     // find the base station !
		followLine(threshold);
		
	stop_motors();

	for (i=0;i<300;i++) {
		set_leds(i);
		_delay_ms(20);
	}
	
	request_delivery(drink_recipes[drink][0],drink_recipes[drink][1],drink_recipes[drink][2],drink_recipes[drink][3]);
    _delay_ms(1000);
	make_u_turn();
	

	while (check_RFID() != station) {  // back to station !
		followLine(threshold);
		set_leds(i++);
	}

		
    stop_motors();  // done
	
}
