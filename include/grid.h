#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

#include "cell.h"

#define NUM_PEERS 20
#define MAX_COMMON_PEERS 13

typedef struct {
    union {
        Cell *cells[81];
        Cell *rows[9][9];
    };
    Cell *cols[9][9];
    Cell *boxes[9][9];
    Cell *peers[81][NUM_PEERS];
    int empty_cells;
} Grid;

typedef enum {
    UNIT_ROW,
    UNIT_COL,
    UNIT_BOX
} UnitType;

Grid *grid_create(char *grid_str);
void grid_destroy(Grid *grid);
bool grid_is_solved(Grid *grid);
void grid_fill_cell(Grid *grid, Cell *cell, int value);
int grid_common_peers(Grid *grid, Cell *cells[], int num_cells, Cell *out[]);

#endif
