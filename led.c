/*
# File:   led.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Setting leds on high/low
*/

#include "pio.h"
#include "led.h"
#define LED_PIO PIO_DEFINE(PORT_C, 2)

// Toggles the led
void led_toggle (void)
{
    pio_output_toggle(LED_PIO);
}

// Switch the LED1 on
void led_on (void)
{
    pio_output_high(LED_PIO);
}


// Switch the LED1 off
void led_off (void)
{
    pio_output_low(LED_PIO);
}


// Initialise LED
void led_init (void)
{
    pio_config_set(LED_PIO, PIO_OUTPUT_LOW);
}
