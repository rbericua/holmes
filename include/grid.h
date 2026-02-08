#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

#include "cell.h"

#define NUM_NEIGHBOURS 20

typedef struct {
    union {
        Cell *cells[81];
        Cell *rows[9][9];
    };
    Cell *cols[9][9];
    Cell *boxes[9][9];
    Cell *neighbours[81][NUM_NEIGHBOURS];
    int empty_cells;
} Grid;

Grid *grid_create(char *grid_str);
void grid_destroy(Grid *grid);
bool grid_is_solved(Grid *grid);
void grid_fill_cell(Grid *grid, Cell *cell, int value);

#endif
