/*
# File:   ircomms.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for ircomms.h
*/

#ifndef IRCOMMS_H
#define IRCOMMS_H

typedef enum
{
    PACKET_NULL,
    PACKET_SHIPS_PLACED,
    PACKET_HITMISS_REQUEST,
    PACKET_HITMISS_RESPONSE,
    PACKET_SHIPS_DESTROYED
} ir_packet_t;

// Sends data in a packet and turns led on when there is activity using the infared
void ir_comms_send (ir_packet_t packet_type, uint8_t data);

// Returns the packet typr received
uint8_t ir_get_incoming_type (void);
// Reads and returns the data of the received packet
uint8_t ir_get_incoming_data (void);

// Get coordinate x from packet received
uint8_t ir_get_incoming_coords_x (void);
// Get coordinate y from packet received
uint8_t ir_get_incoming_coords_y (void);

// Reads a boolean from incoming_data
bool ir_get_incoming_bool (void);

// Sends a hit or miss request with the coordinates x and y
void ir_send_hit_miss_request (uint8_t x, uint8_t y);
// Sends a boolean value, true for a hit and false for a miss
void ir_send_hit_miss_response (bool has_hit);

// Sends a packet to say that ships have been placed
void ir_send_ships_placed (void);

// Set the variables to 0
void ir_clear_inbound_packet (void);

// Handles IR packet transmission and acknowledgement
void ir_comms_tick (void);

#endif
