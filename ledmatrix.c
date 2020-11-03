/*
# File:   ledmatrix.c
# Author: Alexander Miller, Mark Arunchayanon
# Date:   16 Oct 2017
# Descr:  Displays a single column by controlling pins on the led matrix
*/

#include "system.h"
#include "pacer.h"
#include "pio.h"
#include "led.h"

// Define the pins associated with each row
static const pio_t rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
    LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
    LEDMAT_ROW7_PIO
};


// Define the pins associated with each column
static const pio_t cols[] =
{
    LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
    LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};

static bool display_inverted = 0;

void ledmatrix_init (void)
{
    int row;
    for (row = 0; row < LEDMAT_ROWS_NUM; row++) {
        pio_config_set (rows[row], PIO_OUTPUT_HIGH);
    }

    int col;
    for (col = 0; col < LEDMAT_COLS_NUM; col++) {
        pio_config_set (cols[col], PIO_OUTPUT_HIGH);
    }
}

void display_column (uint8_t row_pattern, uint8_t current_column)
{
    uint8_t current_row = 0;
    static int last_column = 0;

    if(display_inverted) {
        row_pattern = ~row_pattern;
    }

    pio_config_set(cols[last_column], PIO_OUTPUT_HIGH);

    while (current_row < 7) {
        if ((row_pattern >> current_row) & 1) {
            pio_config_set(rows[current_row], PIO_OUTPUT_LOW);
        } else {
            pio_config_set(rows[current_row], PIO_OUTPUT_HIGH);
        }
        current_row++;
    }

    pio_config_set(cols[current_column], PIO_OUTPUT_LOW);
    last_column = current_column;
}

void ledmatrix_toggle_invert (void)
{
    display_inverted ^= 1;
}

void ledmatrix_set_invert (bool inverted)
{
    display_inverted = inverted;
}
