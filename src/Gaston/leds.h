
#ifndef LEDS_H
#define LEDS_H

#define LEDS_OFF   0
#define LEDS_RED   1
#define LEDS_GREEN 2
#define LEDS_BLUE  4

void init_leds();
void set_leds(uint8_t col);
void blink_leds(uint8_t count, uint8_t color, uint16_t delay);
void blink_sos() ;

#endif
