/*
# File:   choose_target.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for choose_target.c
*/

#ifndef CHOOSE_TARGET_H
#define CHOOSE_TARGET_H

// Resets the position of the choose target crosshair
void reset_crosshair_position (void);

// Changes the game state to choose target state
void state_choose_target_init (void);

// Displays the crosshair, allows it to be moved around using the navswitch. Sends a hit or miss request with the coordinates when the navswitch is pushed
void state_choose_target_tick (void);



#endif
