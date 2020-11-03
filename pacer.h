/*
# File:   pacer.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for pacer.c
*/

#ifndef PACER_H
#define PACER_H

/* Initialise the pacer module.  */
void pacer_init (uint16_t pacer_frequency);


/* Pace a while loop.  */
void pacer_wait (void);

#endif //PACER_H
