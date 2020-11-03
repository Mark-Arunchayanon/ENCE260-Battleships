/*
# File:   pacer.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Determines te number of ticks for the input frequency
*/

#include <avr/io.h>
#include "pacer.h"

static uint16_t pacer_period;

// Initialise the pacer module
void pacer_init (uint16_t pacer_frequency)
{
    TCNT1 = 0;
    TCCR1A = 0x00;
    TCCR1B = 0x05;
    TCCR1C = 0x00;
    pacer_period = (uint16_t) (1000 * (1 / (float) pacer_frequency) / 0.128);
}


// Waits until the remaining pacer_period is up
void pacer_wait (void)
{
    while(TCNT1 < pacer_period);
    TCNT1 = 0;
}
