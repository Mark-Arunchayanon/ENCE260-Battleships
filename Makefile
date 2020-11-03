# File:   Makefile
# Author: Alexander Miller, Mark Arunchayanon
# Date:   15 Oct 2017
# Descr:  Makefile for ENCE260 Embedded Systems Assignment

# Definitions.
CC = avr-gcc
CFLAGS = -mmcu=atmega32u2 -Os -Wall -Wstrict-prototypes -Wextra -g -I. -I../../drivers/avr -I../../drivers -I../../utils
OBJCOPY = avr-objcopy
SIZE = avr-size
DEL = rm


# Default target.
all: game.out


# Compile: create object files from C source files.
game.o: game.c ../../drivers/avr/system.h ../../drivers/led.h ../../drivers/navswitch.h pacer.h ledmatrix.h led.h bitmap.h ircomms.h
	$(CC) -c $(CFLAGS) $< -o $@

pio.o: ../../drivers/avr/pio.c ../../drivers/avr/pio.h ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

system.o: ../../drivers/avr/system.c ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

led.o: led.c ../../drivers/avr/pio.h ../../drivers/avr/system.h led.h
	$(CC) -c $(CFLAGS) $< -o $@

ledmatrix.o: ledmatrix.c ../../drivers/avr/system.h pacer.h led.h
	$(CC) -c $(CFLAGS) $< -o $@

pacer.o: pacer.c ../../drivers/avr/system.h pacer.h
	$(CC) -c $(CFLAGS) $< -o $@

font.o: ../../utils/font.c ../../drivers/avr/system.h ../../utils/font.h
	$(CC) -c $(CFLAGS) $< -o $@

ircomms.o: ircomms.c led.h
	$(CC) -c $(CFLAGS) $< -o $@

navswitch.o: ../../drivers/navswitch.c ../../drivers/avr/delay.h ../../drivers/avr/pio.h ../../drivers/avr/system.h ../../drivers/navswitch.h
	$(CC) -c $(CFLAGS) $< -o $@

bitmap.o: bitmap.c ../../utils/font.h ../../drivers/avr/system.h ledmatrix.h ../../fonts/font3x5_1.h
	$(CC) -c $(CFLAGS) $< -o $@

ir_uart.o: ../../drivers/avr/ir_uart.c ../../drivers/avr/pio.h ../../drivers/avr/delay.h ../../drivers/avr/system.h ../../drivers/avr/usart1.h ../../drivers/avr/timer0.h
	$(CC) -c $(CFLAGS) $< -o $@

usart1.o: ../../drivers/avr/usart1.c ../../drivers/avr/system.h ../../drivers/avr/usart1.h
	$(CC) -c $(CFLAGS) $< -o $@

timer0.o: ../../drivers/avr/timer0.c ../../drivers/avr/bits.h ../../drivers/avr/prescale.h ../../drivers/avr/system.h ../../drivers/avr/timer0.h
	$(CC) -c $(CFLAGS) $< -o $@

prescale.o: ../../drivers/avr/prescale.c ../../drivers/avr/prescale.h ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

choose_target.o: choose_target.c bitmap.h ircomms.h game.h ../../drivers/avr/system.h ../../drivers/navswitch.h ../../drivers/avr/system.h led.h ../../drivers/avr/ir_uart.h ledmatrix.h
	$(CC) -c $(CFLAGS) $< -o $@

#tinygl.o: ../../utils/tinygl.c ../../drivers/avr/system.h ../../utils/tinygl.h ../../drivers/display.h ../../utils/font.h
#	$(CC) -c $(CFLAGS) $< -o $@


# Link: create ELF output file from object files.

game.out: game.o pio.o system.o led.o ledmatrix.o pacer.o bitmap.o font.o navswitch.o ir_uart.o ircomms.o usart1.o timer0.o prescale.o choose_target.o
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(SIZE) $@


# Target: clean project.
.PHONY: clean
clean:
	-$(DEL) *.o *.out *.hex


# Target: program project.
.PHONY: program
program: game.out
	$(OBJCOPY) -O ihex game.out game.hex
	dfu-programmer atmega32u2 erase; dfu-programmer atmega32u2 flash game.hex; dfu-programmer atmega32u2 start


