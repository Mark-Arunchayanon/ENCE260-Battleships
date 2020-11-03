/*
# File:   choose_target.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Allows the target to be selected using the crosshair, sends a hit or miss request with the coordinates when button is pushed down
*/

#include "system.h"
#include "bitmap.h"
#include "led.h"
#include "ledmatrix.h"
#include "navswitch.h"
#include "ir_uart.h"
#include "ircomms.h"
#include "game.h"

#define SHIP_HIT_FLASH_TICKS 150

static uint8_t crosshair_x = CENTRE_X;
static uint8_t crosshair_y = CENTRE_Y;
static uint8_t last_guessed_x;
static uint8_t last_guessed_y;

void reset_crosshair_position (void)
{
    crosshair_x = CENTRE_X;
    crosshair_y = CENTRE_Y;
    last_guessed_x = 0;
    last_guessed_y = 0;
}

// Changes the game state to choose target state
void state_choose_target_init (void)
{
    set_game_state (STATE_CHOOSE_TARGET);
}

// Displays the crosshair, allows it to be moved around using the navswitch. Sends a hit or miss request with the coordinates when the navswitch is pushed
void state_choose_target_tick (void)
{
    // Setting up the 4 leds that create the crosshair, will not let the center of the crosshair move off the led
    if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
        if (crosshair_x < LEDMAT_ROWS_NUM - 1) {
            crosshair_x++;
        }
    }
    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
        if (crosshair_x > 0) {
            crosshair_x--;
        }
    }
    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
        if (crosshair_y > 0) {
            crosshair_y--;
        }
    }
    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
        if (crosshair_y < LEDMAT_COLS_NUM - 1) {
            crosshair_y++;
        }
    }
    // Sends a hit or miss request with the coordinates if the selected led pin has not been shot at before
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {

        if (!coords_have_been_guessed (crosshair_x, crosshair_y)) {
            last_guessed_x = crosshair_x;
            last_guessed_y = crosshair_y;
            led_on ();
            ir_send_hit_miss_request (last_guessed_x, last_guessed_y);
        }

    }

    if (ir_get_incoming_type () == PACKET_HITMISS_RESPONSE) {

        // Ignore HITMISS_RESPONSE that have already been processed
        if (!coords_have_been_guessed (crosshair_x, crosshair_y)) {

            if (ir_get_incoming_bool ()) {
                set_coords_hitmiss (last_guessed_x, last_guessed_y, 1);
                state_shot_hit_init ();
            } else {
                set_coords_hitmiss (last_guessed_x, last_guessed_y, 0);
                state_shot_miss_init ();
            }

        }

        ir_clear_inbound_packet ();
    }

    uint8_t i;
    uint8_t j;
    static uint8_t flash_tick = SHIP_HIT_FLASH_TICKS;
    static bool flash = 0;
    flash_tick--;
    if(flash_tick == 0){
        flash_tick = SHIP_HIT_FLASH_TICKS;
        flash = !flash;
    }
    for (i = 0; i < LEDMAT_ROWS_NUM; i++) {
        for (j = 0; j < LEDMAT_COLS_NUM; j++) {
            if(coords_have_been_hit (i, j) && flash) bitmap_set_pixel(i, j, 1);
            if(coords_have_been_missed (i, j)) bitmap_set_pixel(i, j, 1);
        }
    }
    // Displays the crosshair, 4
    bitmap_set_pixel (crosshair_x - 1, crosshair_y, LUMINANCE_STEPS);
    bitmap_set_pixel (crosshair_x + 1, crosshair_y, LUMINANCE_STEPS);
    bitmap_set_pixel (crosshair_x, crosshair_y - 1, LUMINANCE_STEPS);
    bitmap_set_pixel (crosshair_x, crosshair_y + 1, LUMINANCE_STEPS);
}
