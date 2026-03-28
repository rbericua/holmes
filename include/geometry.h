#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdbool.h>

#define NUM_PEERS 20

typedef enum {
    UNIT_ROW,
    UNIT_COL,
    UNIT_BOX
} UnitType;

extern int units[3][9][9];
extern int peers[81][NUM_PEERS];

void geometry_init(void);

int cell_unit(int cell, UnitType unit);
int cell_row(int cell);
int cell_col(int cell);
int cell_box(int cell);
int cell_idx_in_box(int cell);
int cell_from_row_col(int row, int col);

bool cells_are_peers(int a, int b);

#endif
