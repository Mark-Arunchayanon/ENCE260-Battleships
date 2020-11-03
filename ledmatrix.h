/*
# File:   ledmatrix.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for ledmatrix.c
*/

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#define LUMINANCE_STEPS 4
#define PWM_RATE 256
#define UPDATE_RATE (LUMINANCE_STEPS * PWM_RATE)
#define LOOP_RATE (7 * UPDATE_RATE)

void ledmatrix_init (void);

void display_column (uint8_t row_pattern, uint8_t current_column);

void ledmatrix_toggle_invert (void);

void ledmatrix_set_invert (bool inverted);

#endif //LEDMATRIX_H
