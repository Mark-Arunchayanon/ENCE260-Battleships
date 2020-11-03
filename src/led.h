/*
# File:   led.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for led.c
*/

#ifndef LED_H
#define LED_H

// Toggles the led
void led_toggle (void);

//switch the LED1 on
void led_on (void);


//switch the LED1 off
void led_off (void);

// Initialise LED
void led_init (void);
#endif
