target = gaston
OBJS = adc.o uart.o drive.o leds.o
port = /dev/ttyACM0

all: $(target).hex

# remove all compiled files
clean:
		rm -f *.hex *.elf *.o
		
#objects from c files
.c.o:
		avr-gcc -mmcu=atmega32u4 -Os -c -g3 $< -o $@
		
#elf file
$(target).elf: $(target).o $(OBJS)
		avr-gcc -mmcu=atmega32u4 -o $@ $^

#hex file
$(target).hex: $(target).elf
		avr-objcopy -j .text -j .data -O ihex $< $@ 
		avr-size $<
		
flash: 
		avrdude -p ATmega32u4 -b 115200 -c avr109 -P $(port) -U flash:w:$(target).hex
		
		
