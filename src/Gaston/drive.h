#ifndef DRIVE_H
#define DRIVE_H


#define AVAILABLE_DRINKS 3

#define NUM_STATIONS 3
#define FROM_BASE    0
#define TO_BASE      1

void init_motors();
void make_u_turn (); 
void followLine (uint16_t threshold); 
void stop_motors() ;
void get_drink(uint8_t station, uint8_t drink, uint16_t threshold) ;
uint16_t get_threshold();
uint8_t cup_present() ;
uint8_t get_direction() ;

#endif
