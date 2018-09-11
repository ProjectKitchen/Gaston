
#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "gaston.h"
#include "adc.h"
#include "uart.h"
#include "leds.h"
#include "drive.h"


#define RIGHT_FORWARD PORTD &= ~(1<<7); PORTE |= (1<<6)
#define RIGHT_BACK    PORTE &= ~(1<<6); PORTD |= (1<<7)
#define RIGHT_STOP    PORTE &= ~(1<<6); PORTD &= ~(1<<7);

#define LEFT_FORWARD  PORTB &= ~(1<<5); PORTB |= (1<<4)
#define LEFT_BACK     PORTB &= ~(1<<4); PORTB |= (1<<5)
#define LEFT_STOP     PORTB &= ~(1<<4); PORTB &= ~(1<<5);

// put in the drink recipies here:
// the 4 time values represent pump on times, range is 0 to 15
// first value is pump1, second value is pump2, ... 
// time values are multiplied by TIME FACTOR:  e.g.: 15 * TIME_FACTOR_WAIT_DELIVERY 1000 = 15 seconds on-time
// caution: if TIME_FACTOR_WAIT_DELIVERY is modified, it needs to be modified also in the source code of the deliverystation.
//

uint8_t drink_recipes[AVAILABLE_DRINKS][4] = {  {10,0,0,0} , {0,10,0,0} , {0,0,10,0} };
char codes[NUM_STATIONS][13]={"4500B8D1FBD7","82003CE0530D", "82003C7EEC2C","82003CA1D6C9"};  // first is the base station (delivery), then the tables !


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


int16_t get_ir_value()
{
	static int16_t buf[AVERAGE_IR]={0};
	static int16_t cnt=0;
	static int32_t sum=0;
	
	sum-=buf[cnt];
	buf[cnt]=read_adc(4);
	sum+=buf[cnt];
	cnt=(cnt+1)%AVERAGE_IR;
	
	return((int16_t)(sum/AVERAGE_IR));
}


extern uint8_t m1,m2;

void followLine (uint16_t threshold) 
{
    uint16_t IR_sense;
    checkbattery();
	IR_sense = get_ir_value();
   
   
	if (IR_sense > threshold+IR_DEADZONE) {
		//LEFT_FORWARD; 
		m2=0; m1=LINEFOLLOW_SPEED; 
		RIGHT_STOP;    
	}
	else if (IR_sense < threshold-IR_DEADZONE) {
		// RIGHT_FORWARD; 
		m1=0; m2=LINEFOLLOW_SPEED;
		LEFT_STOP;    

	} else  {
		//LEFT_FORWARD; 
		//RIGHT_FORWARD;
		m1=LINEFOLLOW_SPEED;m2=LINEFOLLOW_SPEED;
	}
   
}


void stop_motors () 
{
	LEFT_STOP;
	RIGHT_STOP;
	m1=0;
	m2=0;
}


uint16_t get_threshold()
{
    uint16_t threshold,bright_ir_value,dark_ir_value;

    while ((PIND & (1<<4)) !=0 ) 
        bright_ir_value=get_ir_value();
    
    
    set_leds(LEDS_GREEN);  

    // one step calibration: just place robot next to line to get background color
    // the line is assumed to be darker ( difference DEFAULT_THRESHOLD is assumed)
    
    dark_ir_value=DEFAULT_DARK_IR_VALUE;  
    
    //    uncomment for 2-step calibration ( where line color is measured seperately):
    //while ((PINC & (1<<6))!=0)  dark_ir_value=get_ir_value();
    
    threshold= (dark_ir_value+bright_ir_value) / 2;
    return(threshold);
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
                        printDebugMessage(actcode);
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
	int i=0,c=0;
	int waittime;
	
	if (get_direction()==FROM_BASE) make_u_turn();
	
	set_leds(LEDS_BLUE);
	while (check_RFID() != 1)  {   // find the base station !
		followLine(threshold);
		// i++; if (!(i % 5000)) {c++; set_leds(c);}
		if (!cup_present())	{
			 stop_motors();
			 _delay_ms(50);
			 play_sound('c');   // uh ! - we need a cup !
			 while (!cup_present());
		}
	}
		
	stop_motors();
	
	request_delivery(drink_recipes[drink][0],drink_recipes[drink][1],drink_recipes[drink][2],drink_recipes[drink][3]);
    waittime= (drink_recipes[drink][0]+drink_recipes[drink][1]+drink_recipes[drink][2]+drink_recipes[drink][3]);

	for (i=0;i<waittime;i++) {
		set_leds(i);
		_delay_ms(TIME_FACTOR_WAIT_DELIVERY);
	}
	
	set_leds(LEDS_GREEN);

    _delay_ms(1000);
	make_u_turn();
	

	while (check_RFID() != station) {  // back to station !
		followLine(threshold);
		i++; if (!(i % 200)) {c++; set_leds(c);}
	}

	set_leds(LEDS_GREEN);
    stop_motors();  // done	
}


void checkbattery()
{
    static int count=0;
    uint16_t voltage = read_adc(7);

    /*
    if ((count++)%200==0)
    {
        char target[6];
        int_to_str(voltage, target);
        uart_sendstring (target);
        uart_sendstring ("\n");      
    }
    */
    
    if(voltage < BATLOW)
    {        
        count++;
    } else count=0;
    
    if (count>BATLOW_ALERTCOUNTER) {
        stop_motors();
        while(1)
            blink_sos(); //function in leds.c
    }
}
