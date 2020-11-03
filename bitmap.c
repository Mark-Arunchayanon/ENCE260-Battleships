/*
# File:   bitmap.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Allows manipulation of a bitmap to be displayed on led matrix at a variable intensity
*/

#include "system.h"
#include "../../utils/font.h"
#include "ledmatrix.h"
#include <string.h>
#include <ctype.h>

#include "../../fonts/font3x5_1.h"

#define FONT_SCROLL_TICKS 32
static uint8_t bitmap[LEDMAT_ROWS_NUM][LEDMAT_COLS_NUM] = {{0, }};
static int pwm_tick = 0;
static int scroll_tick = FONT_SCROLL_TICKS;

typedef enum
{
    /** Left, center and right aligned text  */
    BITMAP_ALIGN_LEFT,
    BITMAP_ALIGN_CENTER,
    BITMAP_ALIGN_RIGHT
} bitmap_font_align_t;

// Function to be called once per loop to render the bitmap
void bitmap_display (void)
{
    uint8_t x;
    uint8_t row_pattern = 0x00;
    static uint8_t current_column = 0;

    for (x = 0; x < LEDMAT_ROWS_NUM; x++) {
        if (x > 0) row_pattern <<= 1;
        row_pattern |= (bitmap[x][LEDMAT_COLS_NUM-1-current_column] > pwm_tick);
    }

    display_column(row_pattern, current_column);

    current_column = (current_column + 1) % LEDMAT_COLS_NUM;

    pwm_tick++;
    if (pwm_tick >= UPDATE_RATE / PWM_RATE) {
        pwm_tick = 0;
    }
}

// Set an individual pixel in the bitmap
void bitmap_set_pixel (uint8_t x, uint8_t y, uint8_t intensity)
{
    if(x >= LEDMAT_ROWS_NUM || y >= LEDMAT_COLS_NUM) return;
    bitmap[x][y] = intensity;
}

// Returns a single coordinate on the bitmap
uint8_t bitmap_get_pixel (uint8_t x, uint8_t y)
{
    return bitmap[x][y];
}

// Clears the bitmap
void bitmap_clear (void)
{
    int x;
    int y;
    for (x = 0; x < LEDMAT_ROWS_NUM; x++) {
        for (y = 0; y < LEDMAT_COLS_NUM; y++) {
            bitmap_set_pixel(x, y, 0);
        }
    }
}

// Determines the amount of ticks for the amount of time it takes to scroll through the text
int bitmap_get_font_ticks(char* string)
{
    int string_length = strlen(string);
    return FONT_SCROLL_TICKS * (string_length * (FONT_WIDTH + 1) + LEDMAT_ROWS_NUM);
}

// Resets the scroll tick
void bitmap_reset_font_scroll (void)
{
    scroll_tick = 0;
}

//
void bitmap_render_font (char* string, uint8_t pos_x, uint8_t pos_y, bitmap_font_align_t align, int scroll)
{
    uint8_t x;
    uint8_t y;
    uint8_t string_length = strlen(string);
    uint8_t i = 0;

    while(string[i] != '\0') {
        for(x = 0; x < FONT_WIDTH; x++) {
            for(y = 0; y < FONT_HEIGHT; y++){
                int offset_x = x + i * (FONT_WIDTH + 1);

                if(align == BITMAP_ALIGN_CENTER) {
                    offset_x -= string_length * (FONT_WIDTH + 1) / 2;
                } else if(align == BITMAP_ALIGN_RIGHT) {
                    offset_x -= string_length * (FONT_WIDTH + 1);
                }

                int draw_x = pos_x + offset_x + LEDMAT_ROWS_NUM - scroll_tick / 32;
                int draw_y = pos_y + y;

                if(draw_x < 0 || draw_x >= LEDMAT_ROWS_NUM || draw_y >= LEDMAT_COLS_NUM || draw_y < 0) {
                    continue;
                }

                bool pixel_on = font_pixel_get(&font3x5_1, toupper(string[i]), x, FONT_HEIGHT-1-y);
                if(pixel_on) bitmap_set_pixel(draw_x, draw_y, (pixel_on ? LUMINANCE_STEPS : 0));
            }
        }
        i++;
    }

    if(scroll) scroll_tick++;

    if(scroll_tick == bitmap_get_font_ticks(string)){
        scroll_tick = 0;
    }
}
