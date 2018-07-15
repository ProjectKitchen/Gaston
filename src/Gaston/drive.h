#ifndef DRIVE_H
#define DRIVE_H


void init_motors();
void turn (); 
void followLine (uint16_t threshold); 
void stop () ;
uint16_t get_threshold();


#endif
