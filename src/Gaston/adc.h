
#ifndef ADC_H
#define ADC_H


void init_adc (void);
uint16_t read_adc (uint16_t channel );
void int_to_str (uint16_t val, char * target) ;

#endif