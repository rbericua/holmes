#ifndef GRID_H
#define GRID_H

#include <stdbool.h>

typedef struct {
    int values[81];
    unsigned int cands[81];
    bool clues[81];
    int num_empty;
} Grid;

Grid *grid_create(char *grid_str);
void grid_destroy(Grid *grid);
bool grid_is_solved(Grid *grid);
void grid_fill_cell(Grid *grid, int cell, int value);

int grid_cell_value(Grid *grid, int cell);
int grid_cell_cands(Grid *grid, int cell);
int grid_cell_num_cands(Grid *grid, int cell);
bool grid_cell_is_clue(Grid *grid, int cell);
bool grid_cell_is_empty(Grid *grid, int cell);
bool grid_cell_has_cand(Grid *grid, int cell, int cand);
void grid_cell_add_cand(Grid *grid, int cell, int cand);
void grid_cell_add_cands(Grid *grid, int cell, unsigned int cands);
void grid_cell_remove_cand(Grid *grid, int cell, int cand);
void grid_cell_remove_cands(Grid *grid, int cell, unsigned int cands);
void grid_cell_clear_cands(Grid *grid, int cell);
int grid_cell_first_cand(Grid *grid, int cell);

unsigned int grid_region_cands_intersection(Grid *grid, int region[],
                                            int region_len);
unsigned int grid_region_cands_union(Grid *grid, int region[], int region_len);
int grid_region_missing_values(Grid *grid, int region[], int region_len,
                               int out[]);
int grid_region_with_cand(Grid *grid, int region[], int region_len, int cand,
                          int out[]);
int grid_region_with_n_cands_max(Grid *grid, int region[], int region_len,
                                 int n, int out[]);

#endif
