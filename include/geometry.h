#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdbool.h>

#define NUM_PEERS 20
#define MAX_COMMON_PEERS 13

typedef enum {
    UNIT_ROW,
    UNIT_COL,
    UNIT_BOX
} UnitType;

typedef enum {
    LINK_WEAK = 1 << 0,
    LINK_STRONG = 1 << 1,
    LINK_DUAL = LINK_WEAK | LINK_STRONG,
} LinkType;

extern int units[3][9][9];
extern int peers[81][NUM_PEERS];

void geometry_init(void);

int cell_unit(int cell, UnitType unit);
int cell_row(int cell);
int cell_col(int cell);
int cell_box(int cell);
int cell_idx_in_box(int cell);
int cell_from_row_col(int row, int col);
int cell_from_unit_pos(int unit, int pos, UnitType unit_type);

bool cells_are_peers(int a, int b);
bool cells_in_same_row(int cells[], int num_cells);
bool cells_in_same_col(int cells[], int num_cells);
bool cells_in_same_box(int cells[], int num_cells);
int cells_common_peers(int cells[], int num_cells, int out[]);

UnitType other_line(UnitType unit);

#endif
