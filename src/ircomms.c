/*
# File:   ircomms.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Allows reliable transmissions of boolean and coordinate values over ir
*/

#include "ir_uart.h"
#include "ircomms.h"
#include "led.h"

#define ID_BITS      0xB0
#define ID_MASK      0xF0
#define TYPE_MASK    ~ID_MASK

#define ACK_BITS     0xAC

#define DATA_ID_BITS 0x40
#define DATA_ID_MASK 0b11000000
#define DATA_MASK    ~DATA_ID_MASK

static uint8_t outbound_packet_type_bits = 0;
static uint8_t outbound_data_bits = 0;

static ir_packet_t inbound_packet_type = 0;
static uint8_t inbound_data = 0;
static bool inbound_ready = 0;

static uint8_t comms_tick = 0;

// Sends data in a packet and turns led on when there is activity using the infared
void ir_comms_send (ir_packet_t packet_type, uint8_t data)
{
    outbound_packet_type_bits = ID_BITS | (packet_type & 0x0F);
    outbound_data_bits = (DATA_ID_BITS & DATA_ID_MASK) | data;
    comms_tick = 0;
    led_on ();
}

// Returns the packet typr received
uint8_t ir_get_incoming_type (void)
{
    if(!inbound_ready) return 0;
    return inbound_packet_type;
}

// Reads and returns the data of the received packet
uint8_t ir_get_incoming_data (void)
{
    if(!inbound_ready) return 0;
    return inbound_data & DATA_MASK;
}

// Get coordinate x from packet received
uint8_t ir_get_incoming_coords_x (void)
{
    if(!inbound_ready) return 0;
    return (uint8_t) (inbound_data >> 3);
}

// Get coordinate y from packet received
uint8_t ir_get_incoming_coords_y (void)
{
    if(!inbound_ready) return 0;
    return (uint8_t) (inbound_data & 0x07);
}

// Reads a boolean from incoming_data
bool ir_get_incoming_bool (void) {
    return ((bool) ir_get_incoming_data () & 0x01);
}

// Sends a hit or miss request with the coordinates x and y
void ir_send_hit_miss_request (uint8_t x, uint8_t y) {
    // Byte layout for coords data: (IDxxxyyy)
    uint8_t coords_data = (x << 3) | (y & 0x07);
    ir_comms_send (PACKET_HITMISS_REQUEST, coords_data);
}

// Sends a boolean value, true for a hit and false for a miss
void ir_send_hit_miss_response (bool has_hit) {
    // Byte layout for bool data: (ID111111) or (ID000000)
    uint8_t data_bits = 0x3F;
    if (!has_hit) data_bits = 0x00;
    ir_comms_send (PACKET_HITMISS_RESPONSE, data_bits);
}

// Sends a packet to say that ships have been placed
void ir_send_ships_placed (void) {
    ir_comms_send (PACKET_SHIPS_PLACED, 1);
}

// Set the variables to 0
void ir_clear_inbound_packet (void)
{
    inbound_ready = 0;
    inbound_data = 0;
    inbound_packet_type = 0;
}

// Sends an acknowledgment and sets inbound ready to 1
void ir_send_ack (void)
{
    inbound_ready = 1;
    ir_uart_putc (ACK_BITS);
}

// Handles IR packet transmission and acknowledgement
void ir_comms_tick (void)
{
    if(ir_uart_read_ready_p ()) {
        uint8_t recv_data = (uint8_t) ir_uart_getc ();
        if(recv_data == ACK_BITS) {
            // Acknowledgement Packet has been received, stop sending type/data
            outbound_packet_type_bits = 0;
            outbound_data_bits = 0;
            led_off ();
        } else if((recv_data & ID_MASK) == ID_BITS) {
            // Type Packet has been received, set inbound packet type
            inbound_packet_type = (ir_packet_t) (recv_data & TYPE_MASK);
        } else if(( (recv_data & DATA_ID_MASK) == DATA_ID_BITS) && inbound_packet_type){
            // Data Packet has been received and packet type received, process inbound data
            inbound_data = recv_data & DATA_MASK;
            if (inbound_packet_type == PACKET_HITMISS_RESPONSE) {
                // Process inbound boolean value
                uint8_t on_count = 0;
                uint8_t i = 0;
                for(i = 0; i < 6; i++) {
                    on_count += ((inbound_data >> i) & 1);
                }
                // Use redundant bits to determine need for retransmission
                inbound_data = (on_count > 3);
                if (on_count != 3){
                    ir_send_ack ();
                }
            } else {
                ir_send_ack ();
            }
        }
    }

    if(!outbound_packet_type_bits) return;

    if(comms_tick % 28 == 0) {
        ir_uart_putc (outbound_packet_type_bits);
        comms_tick = 0;
    } else if (comms_tick == 14) {
        ir_uart_putc (outbound_data_bits);
    }

    comms_tick++;
}
