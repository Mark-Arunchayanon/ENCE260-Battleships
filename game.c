/*
# File:   game.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Contains the main game logic for Battleship
*/

#include "system.h"
#include "bitmap.h"
#include "pacer.h"
#include <math.h>
#include <stdlib.h>
#include "led.h"
#include "ledmatrix.h"
#include "navswitch.h"
#include "game.h"
#include "ir_uart.h"
#include "ircomms.h"
#include "choose_target.h"

static PlayerShip player_ships[SHIPS_COUNT] = {{0, 0, 4, 1, 0}, {0, 0, 3, 1, 0}, {0, 0, 3, 1, 0}};

static bool hits_map[LEDMAT_ROWS_NUM][LEDMAT_COLS_NUM] = {{0, }};
static bool misses_map[LEDMAT_ROWS_NUM][LEDMAT_COLS_NUM] = {{0, }};

static game_state_t game_state;
static bool enemy_has_placed_ships = 0;
static bool is_player_turn = 0;

static uint8_t my_hit_count = 0;
static uint8_t enemy_hit_count = 0;

static int anim_ticks = 0;

// Initialises the variables and resets ship placements, hits and misses count
void game_init (void)
{
    state_intro_explosion_init ();

    // Reset ship placements
    uint8_t i;
    for (i = 0; i < SHIPS_COUNT; i++) {
        player_ships[i].placed = 0;
        player_ships[i].vertical = 1;
    }

    // Reset hits and misses
    uint8_t j;
    for (i = 0; i < LEDMAT_ROWS_NUM; i++) {
        for (j = 0; j< LEDMAT_COLS_NUM; j++) {
            hits_map[i][j] = 0;
            misses_map[i][j] = 0;
        }
    }

    enemy_has_placed_ships = 0;
    is_player_turn = 0;
    my_hit_count = 0;
    enemy_hit_count = 0;
    reset_crosshair_position ();
}

// Allows other modules to change the game state
void set_game_state (game_state_t state)
{
    game_state = state;
}

// Checks if a particular coordinate has been guessed
bool coords_have_been_guessed (uint8_t x, uint8_t y)
{
    return !hits_map[x][y] && !misses_map[x][y];
}

// Checks if a ship was hit by the player at x, y
bool coords_have_been_hit (uint8_t x, uint8_t y)
{
    return hits_map[x][y];
}


// Checks if a shot the player missed at x, y
bool coords_have_been_missed (uint8_t x, uint8_t y)
{
    return misses_map[x][y];
}


// Sets the hit/miss status of a coordinate
void set_coords_hitmiss (uint8_t x, uint8_t y, bool hit)
{
    if (hit) {
        hits_map[x][y] = 1;
    } else {
        misses_map[x][y] = 1;
    }
}

// Changes game state to ship rotate state
void state_place_ship_rotate_init (void)
{
    game_state = STATE_PLACE_SHIP_ROTATE;
}

// Allows ships to be rotated vertically or horizontally,
// push up or down on the navswitch to make the ship vertical and left or right for horizantal
void state_place_ship_rotate_tick (void)
{
    uint8_t i;
    // Find the first unplaced ship
    bool found_unplaced = 0;
    for (i = 0; i < SHIPS_COUNT; i++) {
        PlayerShip current_ship = player_ships[i];
        if (!current_ship.placed && !found_unplaced) {
            found_unplaced = 1;

            int half_length = (int)(current_ship.length / 2);
            current_ship.x = CENTRE_X;
            current_ship.y = CENTRE_Y;

            if (navswitch_push_event_p (NAVSWITCH_WEST) || navswitch_push_event_p (NAVSWITCH_EAST)) {
                player_ships[i].vertical = 1;
            }

            if (navswitch_push_event_p (NAVSWITCH_NORTH) || navswitch_push_event_p (NAVSWITCH_SOUTH)) {
                player_ships[i].vertical = 0;
            }

            if (player_ships[i].vertical) {
                current_ship.y -= half_length;
            } else {
                current_ship.x -= half_length;
            }

            if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
                player_ships[i].x = current_ship.x;
                player_ships[i].y = current_ship.y;
                state_place_ship_move_init ();
                break;
            }

            player_ship_render (current_ship, 1);
        }
        if (current_ship.placed) player_ship_render (current_ship, 0);
    }
    // Determine which player goes first, the player who places all the ships first gets to start
    if (!found_unplaced) {
        if(enemy_has_placed_ships) {
            state_waiting_turn_init ();
        } else {
            is_player_turn = 1;
            state_choose_target_init ();
        }

        ir_send_ships_placed ();
    }
}

// Changes game state to placing ship state
void state_place_ship_move_init (void)
{
    game_state = STATE_PLACE_SHIP_MOVE;
}

// Allows ships to be moved around the led matrix. Will not let you place a ship it will intersect with another ship
void state_place_ship_move_tick (void)
{
    uint8_t i;
    // Find the first unplaced ship
    bool found_unplaced = 0;
    for (i = 0; i < SHIPS_COUNT; i++) {
        PlayerShip current_ship = player_ships[i];
        if (!current_ship.placed && !found_unplaced) {
            found_unplaced = 1;
            // Allows ships to be moved using the navswitch
            if (navswitch_push_event_p (NAVSWITCH_NORTH)
                && current_ship.x + (!current_ship.vertical ? current_ship.length - 1 : 0) < LEDMAT_ROWS_NUM - 1) {
                player_ships[i].x += 1;
            }

            if (navswitch_push_event_p (NAVSWITCH_EAST) && current_ship.y > 0) {
                player_ships[i].y -= 1;
            }

            if (navswitch_push_event_p (NAVSWITCH_SOUTH) && current_ship.x > 0) {
                player_ships[i].x -= 1;
            }

            if (navswitch_push_event_p (NAVSWITCH_WEST)
                && current_ship.y + (current_ship.vertical ? current_ship.length - 1 : 0) < LEDMAT_COLS_NUM - 1) {
                player_ships[i].y += 1;
            }
            // Confirms the placement of the ship if navswitch is pushed, will not allow ships to overlap
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                uint8_t j = 0;
                bool can_place = 1;
                for(j = 0; j < SHIPS_COUNT && can_place; j++) {
                    if(player_ships[j].placed && ship_intersects_with_ship(player_ships[j], current_ship)) {
                        can_place = 0;
                    }
                }
                if(can_place) {
                    player_ships[i].placed = 1;
                    state_place_ship_rotate_init ();
                    break;
                }
            }
            player_ship_render (current_ship, 1);
        }
        if(current_ship.placed) player_ship_render (current_ship, 0);
    }
}

// Renders an individual player ship to the bitmap
void player_ship_render (PlayerShip ship, bool do_flash)
{
    static uint8_t flash_tick = SHIP_PLACEMENT_FLASH_TICKS;
    if (do_flash) flash_tick--;
    if (flash_tick == 0) flash_tick = SHIP_PLACEMENT_FLASH_TICKS;

    uint8_t pixel = (flash_tick < SHIP_PLACEMENT_FLASH_TICKS / 2 && do_flash  ? 0 : LUMINANCE_STEPS);

    if(ship.vertical) {
        int y;
        for (y = 0; y < ship.length; y++) {
            bitmap_set_pixel (ship.x, y + ship.y, pixel);
        }
    } else {
        int x;
        for (x = 0; x < ship.length; x++) {
            bitmap_set_pixel (x + ship.x, ship.y, pixel);
        }
    }
}


// Sets the amount of ticks for the animation and sets the game state to intro explosion state
void state_intro_explosion_init (void)
{
    anim_ticks = EXPLOSION_ANIMATION_TICKS;
    game_state = STATE_INTRO_EXPLOSION;
}

// Displays the explosion 3 times then displays the intro text, if button is pushed down, game state will change to rotating ship state
void state_intro_explosion_tick (void)
{
    uint8_t x;
    uint8_t y;
    const uint8_t levels[] = {0, 0, 0, 0, 2, 4, 4};

    for (x = 0; x < LEDMAT_ROWS_NUM; x++) {
        for (y = 0; y < LEDMAT_COLS_NUM; y++) {
            uint8_t dx = abs (x - CENTRE_X);
            uint8_t dy = abs (y - CENTRE_Y);
            uint8_t r = dx + dy;
            bitmap_set_pixel (x, y, levels[(int)(r + (float) anim_ticks / 60) % 7]);
        }
    }
    // If navswitch is pushed, it will stop the explosion and change to ship rotate state
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        return state_place_ship_rotate_init ();
    }
    // If nothing has been pushed during the 3 explosions, it will change to display intro text
    anim_ticks--;
    if(anim_ticks == 0) state_intro_text_init ();
}

// Determines the amount of ticks for the intro text, changes game state to intro text state and resets the scroll
void state_intro_text_init (void)
{
    anim_ticks = bitmap_get_font_ticks ("BattleShip!");
    game_state = STATE_INTRO_TEXT;
    bitmap_reset_font_scroll ();
}

// Displays a scrolling intro text (Name of the game and instruction to start the game), if navswitch is pushed it will change to ship rotation selection state
void state_intro_text_tick (void)
{
    static bool instruction_shown = 0;

    bitmap_render_font ((instruction_shown ? "Push to start" : "BattleShip!"), 0, 0, BITMAP_ALIGN_LEFT, 1);

    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        instruction_shown = 0;
        return state_place_ship_rotate_init ();
    }

    anim_ticks--;
    if (anim_ticks == 0) {
        if(instruction_shown) {
            instruction_shown = 0;
            state_intro_explosion_init ();
        } else {
            instruction_shown = 1;
            anim_ticks = bitmap_get_font_ticks ("Push to start");
        }
    }

}

// Counts the length of all ships and returns the total length.
uint8_t get_total_ship_length (void)
{
    uint8_t i = 0;
    uint8_t total = 0;
    for (i = 0; i< SHIPS_COUNT; i++) {
        total += player_ships[i].length;
    }
    return total;
}

// Changes the game state to shot hit state, gets the amount of ticks needed for text to scroll across and increments shot hit count
void state_shot_hit_init (void)
{
    led_off ();
    game_state = STATE_SHOT_HIT;
    anim_ticks = bitmap_get_font_ticks ("HIT!");
    bitmap_reset_font_scroll ();

    if (is_player_turn) {
        my_hit_count++;
    } else {
        enemy_hit_count++;
    }
}

// Displays a scrolling text (HIT!) and changes to the other player's turn
void state_shot_hit_tick (void)
{
    bitmap_render_font ("HIT!", 0, 0, BITMAP_ALIGN_LEFT, 1);
    anim_ticks--;
    if (anim_ticks == 0) player_turn_toggle ();
}

// Changes to the other player's turn, also puts the current player to waiting state
void player_turn_toggle (void)
{
    uint8_t total_length = get_total_ship_length ();

    if (my_hit_count == total_length) {
        return state_won_init ();
    } else if (enemy_hit_count == total_length) {
        return state_lost_init ();
    }

    is_player_turn = !is_player_turn;

    if (is_player_turn) {
        state_choose_target_init ();
    } else {
        state_waiting_turn_init ();
    }
}

// Changes game state to shot miss state, determines the amount of ticks needed for the text
void state_shot_miss_init (void)
{
    game_state = STATE_SHOT_MISS;
    anim_ticks = bitmap_get_font_ticks ("MISS!");
    bitmap_reset_font_scroll ();
}

// Displays a scrolling text (MISS!) and changes to the other player's turn
void state_shot_miss_tick (void)
{
    bitmap_render_font ("MISS!", 0, 0, BITMAP_ALIGN_LEFT, 1);
    anim_ticks--;
    if (anim_ticks == 0) player_turn_toggle ();
}

// True if the ships that are about to be placed intersect with a point of another ship, otherwise false
bool ship_intersects_with_point (PlayerShip ship, uint8_t x, uint8_t y)
{
    uint8_t j;
    for (j = 0; j < ship.length; j++) {
        if (ship.x + (ship.vertical ? 0 : j) == x
            && ship.y + (ship.vertical ? j : 0) == y) {
            return 1;
        }
    }
    return 0;
}

// True if the ships that are about to be placed intersect with another ship ,otherwise false
bool ship_intersects_with_ship (PlayerShip ship1, PlayerShip ship2)
{
    uint8_t j;
    for(j = 0; j < ship1.length; j++) {
        uint8_t x = ship1.x + (ship1.vertical ? 0 : j);
        uint8_t y = ship1.y + (ship1.vertical ? j : 0);
        if (ship_intersects_with_point (ship2, x, y)) {
            return 1;
        }
    }
    return 0;
}

// Changes game state to waiting state
void state_waiting_turn_init (void)
{
    game_state = STATE_WAITING_TURN;
    bitmap_reset_font_scroll ();
}

// Displays scrolling text (WAITING..), waits for a hit or miss request and takes in the coordinates to see if its a hit or miss
// then changes the game state of both fun kits to shot hit state or shot miss state
void state_waiting_turn_tick (void)
{
    bitmap_render_font ("Waiting..", 0, 0, BITMAP_ALIGN_LEFT, 1);

    if (ir_get_incoming_type () == PACKET_HITMISS_REQUEST) {
        uint8_t target_x = ir_get_incoming_coords_x ();
        uint8_t target_y = ir_get_incoming_coords_y ();
        ir_clear_inbound_packet ();

        uint8_t i;
        bool has_hit_ship = 0;
        for (i = 0; i < SHIPS_COUNT && !has_hit_ship; i++) {
            if (ship_intersects_with_point (player_ships[i], target_x, target_y)) has_hit_ship = 1;
        }

        ir_send_hit_miss_response (has_hit_ship);

        if (has_hit_ship) {
            state_shot_hit_init ();
        } else {
            state_shot_miss_init ();
        }
    }
}

// Checks if enemy has placed all their ships and clears inbound packet
void check_enemy_placement (void)
{
    if (ir_get_incoming_type () == PACKET_SHIPS_PLACED && ir_get_incoming_bool ()) {
        enemy_has_placed_ships = 1;
        ir_clear_inbound_packet ();
    }
}

// Changes game state to won state and reset font scroll
void state_won_init (void)
{
    game_state = STATE_WON;
    bitmap_reset_font_scroll ();
}

// Displays scrolling text (WINNER!), if navswitch is pushed it will restart the game
void state_won_tick (void)
{
    bitmap_render_font ("WINNER!", 0, 0, BITMAP_ALIGN_LEFT, 1);
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) game_init ();
}

// Changes game state to lose state and reset font scroll
void state_lost_init (void)
{
    game_state = STATE_LOST;
    bitmap_reset_font_scroll ();
}

// Displays scrolling text (LOSER!), if navswitch is pushed it will restart the game
void state_lost_tick (void)
{
    bitmap_render_font ("LOSER!", 0, 0, BITMAP_ALIGN_LEFT, 1);
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) game_init ();
}

// Initialises functions
int main (void)
{
    // Initialise led, pacer and ir functions
    system_init ();
    ledmatrix_init ();
    led_init ();
    navswitch_init ();
    pacer_init (LOOP_RATE);
    ir_uart_init ();

    game_init ();

    //Infinite while loop to check which state the fun kits are in and performs the functions
    while (1) {
        pacer_wait ();
        bitmap_clear ();
        navswitch_update ();
        ir_comms_tick ();

        if (!enemy_has_placed_ships) check_enemy_placement ();

        if (game_state == STATE_INTRO_EXPLOSION) {
            state_intro_explosion_tick ();

        } else if (game_state == STATE_INTRO_TEXT) {
            state_intro_text_tick ();

        } else if (game_state == STATE_PLACE_SHIP_ROTATE) {
            state_place_ship_rotate_tick ();

        } else if (game_state == STATE_PLACE_SHIP_MOVE) {
            state_place_ship_move_tick ();

        } else if (game_state == STATE_CHOOSE_TARGET) {
            state_choose_target_tick ();

        } else if (game_state == STATE_SHOT_HIT) {
            state_shot_hit_tick ();

        } else if (game_state == STATE_SHOT_MISS) {
            state_shot_miss_tick ();

        } else if (game_state == STATE_WAITING_TURN) {
            state_waiting_turn_tick ();

        } else if (game_state == STATE_WON) {
            state_won_tick ();

        } else if (game_state == STATE_LOST) {
            state_lost_tick ();
        }


        bitmap_display();
    }

    return 0;
}
