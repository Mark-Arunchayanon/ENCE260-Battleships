/*
# File:   bitmap.h
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Header file for bitmap.c
*/

#ifndef BITMAP_H
#define BITMAP_H

typedef enum
{
    /** Left, center and right aligned text  */
    BITMAP_ALIGN_LEFT,
    BITMAP_ALIGN_CENTER,
    BITMAP_ALIGN_RIGHT
} bitmap_font_align_t;

// Function to be called once per loop to render the bitmap
void bitmap_display (void);

// Clears the bitmap
void bitmap_clear (void);

// Set an individual pixel in the bitmap
void bitmap_set_pixel (uint8_t x, uint8_t y, uint8_t intensity);

// Returns a single coordinate on the bitmap
uint8_t bitmap_get_pixel (uint8_t x, uint8_t y);

// Determines the amount of ticks for the amount of time it takes to scroll through the text
int bitmap_get_font_ticks(char* string);

// Resets the scroll tick
void bitmap_reset_font_scroll (void);

//
void bitmap_render_font (char* string, uint8_t pos_x, uint8_t pos_y, bitmap_font_align_t align, bool scrolling);

#endif
