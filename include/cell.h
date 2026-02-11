#ifndef CELL_H
#define CELL_H

#include <stdbool.h>

#include "cand_set.h"

#define ROW_FROM_IDX(i) ((i) / 9)
#define COL_FROM_IDX(i) ((i) % 9)
#define BOX_FROM_IDX(i) ((ROW_FROM_IDX(i) / 3) * 3 + COL_FROM_IDX(i) / 3)
#define BOX_POSITION_FROM_IDX(i) \
    ((ROW_FROM_IDX(i) % 3) * 3 + COL_FROM_IDX(i) % 3)

#define IDX_FROM_ROW_COL(r, c) ((r) * 9 + c)

typedef struct {
    int value;
    CandSet cands;
    int row;
    int col;
    int box;
} Cell;

Cell *cell_create(int idx, int value);
void cell_destroy(Cell *cell);
int cell_idx(Cell *cell);
bool cell_is_empty(Cell *cell);
bool cell_eq(Cell *a, Cell *b);
bool cell_is_peer(Cell *a, Cell *b);
bool cell_has_cand(Cell *cell, int cand);
void cell_add_cand(Cell *cell, int cand);
void cell_remove_cand(Cell *cell, int cand);
void cell_remove_cands(Cell *cell, CandSet cands);
void cell_clear_cands(Cell *cell);
int cell_only_cand(Cell *cell);

void cells_idxs(Cell *cells[], int num_cells, int out[]);
CandSet cells_missing_values_to_set(Cell *cells[], int num_cells);
int cells_missing_values_to_arr(Cell *cells[], int num_cells, int out[]);
CandSet cells_cand_intersection(Cell *cells[], int num_cells);
CandSet cells_cand_union(Cell *cells[], int num_cells);
int cells_with_cand(Cell *cells[], int num_cells, int cand, Cell *out[]);
int cells_with_cands_some(Cell *cells[], int num_cells, CandSet cands,
                          Cell *out[]);
int cells_with_n_cands_max(Cell *cells[], int num_cells, int n, Cell *out[]);
int cells_with_removals(Cell *cells[], int num_cells, CandSet cands,
                        Cell *out_cells[], CandSet out_cands[]);

#endif
