/*
# File:   game.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for game.c
*/

#ifndef GAME_H
#define GAME_H

#define SHIPS_COUNT 3
#define CENTRE_X 3
#define CENTRE_Y 2
#define EXPLOSION_ANIMATION_TICKS 1200
#define SHIP_PLACEMENT_FLASH_TICKS 250

struct ship_s{
    uint8_t x;
    uint8_t y;
    uint8_t length;
    bool vertical;
    bool placed;
};

typedef struct ship_s PlayerShip;

typedef enum
{
    STATE_INTRO_EXPLOSION,      // Explosion Animation
    STATE_INTRO_TEXT,           // BattleShips title text
    STATE_PLACE_SHIP_ROTATE,    // Select ship orientation
    STATE_PLACE_SHIP_MOVE,      // Select ship location
    STATE_CHOOSE_TARGET,        // Choose a coord to fire at
    STATE_SHOT_HIT,             // Shot success message
    STATE_SHOT_MISS,            // Shot missed message
    STATE_WAITING_TURN,         // Waiting for the other player to fire
    STATE_WON,                  // Won game screen
    STATE_LOST                  // Lost game screen
} game_state_t;

// Initialises the variables and resets ship placements, hits and misses count
void game_init (void);

// Allows other modules to change the game state
void set_game_state (game_state_t state);

// Checks if a particular coordinate has been guessed
bool coords_have_been_guessed (uint8_t x, uint8_t y);

// Checks if a ship was hit by the player at x, y
bool coords_have_been_hit (uint8_t x, uint8_t y);

// Checks if a shot by the player missed at x, y
bool coords_have_been_missed (uint8_t x, uint8_t y);

// Sets the hit/miss status of a coordinate
void set_coords_hitmiss (uint8_t x, uint8_t y, bool hit);

// Changes game state to ship roatate state
// Allows ships to be rotated vertically or horizontally,
// push up or down on the navswitch to make the ship vertical and left or right for horizantal
void state_place_ship_rotate_init (void);
void state_place_ship_rotate_tick (void);

// Changes game state to placing ship state
// Allows ships to be moved around the led matrix. Will not let you place a ship it will intersect with another ship
// Renders an individual player ship to the bitmap
void state_place_ship_move_init (void);
void state_place_ship_move_tick (void);
void player_ship_render (PlayerShip ship, bool do_flash);

// Sets the amount of ticks for the animation and sets the game state to intro explosion state
// Displays the explosion 3 times then displays the intro text, if button is pushed down, game state will change to rotating ship state
void state_intro_explosion_init (void);
void state_intro_explosion_tick (void);

// Determines the amount of ticks for the intro text, changes game state to intro text state and resets the scroll
// Displays a scrolling intro text (Name of the game and instruction to start the game), if navswitch is pushed it will change to ship rotation selection state
void state_intro_text_init (void);
void state_intro_text_tick (void);

// Changes the game state to shot hit state, gets the amount of ticks needed for text to scroll across and increments shot hit count
// Displays a scrolling text (HIT!) and changes to the other player's turn
// Changes to the other player's turn, also puts the current player to waiting state
void state_shot_hit_init (void);
void state_shot_hit_tick (void);
void player_turn_toggle (void);

// Changes game state to shot miss state, determines the amount of ticks needed for the text
// Displays a scrolling text (MISS!) and changes to the other player's turn
void state_shot_miss_init (void);
void state_shot_miss_tick (void);

// True if the ships that are about to be placed intersect with a point of another ship, otherwise false
// True if the ships that are about to be placed intersect with another ship ,otherwise false
bool ship_intersects_with_point (PlayerShip ship, uint8_t x, uint8_t y);
bool ship_intersects_with_ship (PlayerShip ship1, PlayerShip ship2);

// Changes game state to waiting state
// Displays scrolling text (WAITING..), waits for a hit or miss request and takes in the coordinates to see if its a hit or miss
// then changes the game state of both fun kits to shot hit state or shot miss state
void state_waiting_turn_init (void);
void state_waiting_turn_tick (void);

// Changes game state to lose state and reset font scroll
// Displays scrolling text (LOSER!), if navswitch is pushed it will restart the game
void state_lost_init (void);
void state_lost_tick (void);

// Changes game state to won state and reset font scroll
// Displays scrolling text (WINNER!), if navswitch is pushed it will restart the game
void state_won_init (void);
void state_won_tick (void);

// Checks if enemy has placed all their ships and clears inbound packet
void check_enemy_placement (void);

int main (void);
#endif
