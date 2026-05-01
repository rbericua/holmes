#include "grid.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "geometry.h"

static Grid *grid_from_values(char *grid_str);
static Grid *grid_from_cands(char *grid_str);

Grid *grid_create(char *grid_str) {
    Grid *grid;

    if (strncmp(grid_str, "S9B", 3) == 0) {
        grid = grid_from_cands(&grid_str[3]);
    } else {
        grid = grid_from_values(grid_str);
    }

    return grid;
}

void grid_destroy(Grid *grid) {
    free(grid);
}

bool grid_is_solved(Grid *grid) {
    return grid->num_empty == 0;
}

void grid_fill_cell(Grid *grid, int cell, int value) {
    grid->values[cell] = value;
    grid_cell_clear_cands(grid, cell);
    grid->num_empty--;

    for (int i = 0; i < NUM_PEERS; i++) {
        grid_cell_remove_cand(grid, peers[cell][i], value);
    }
}

void grid_clear_cell(Grid *grid, int cell, CandSet cands) {
    grid->values[cell] = 0;
    grid->cands[cell] = cands;
    grid->num_empty++;
}

int grid_cell_value(Grid *grid, int cell) {
    return grid->values[cell];
}

int grid_cell_cands(Grid *grid, int cell) {
    return grid->cands[cell];
}

int grid_cell_num_cands(Grid *grid, int cell) {
    return cand_set_len(grid->cands[cell]);
}

bool grid_cell_is_clue(Grid *grid, int cell) {
    return grid->clues[cell];
}

bool grid_cell_is_empty(Grid *grid, int cell) {
    return grid->values[cell] == 0;
}

bool grid_cell_has_cand(Grid *grid, int cell, int cand) {
    return cand_set_has(grid->cands[cell], cand);
}

void grid_cell_add_cand(Grid *grid, int cell, int cand) {
    cand_set_add(&grid->cands[cell], cand);
}

void grid_cell_add_cands(Grid *grid, int cell, CandSet cands) {
    grid->cands[cell] |= cands;
}

void grid_cell_remove_cand(Grid *grid, int cell, int cand) {
    cand_set_remove(&grid->cands[cell], cand);
}

void grid_cell_remove_cands(Grid *grid, int cell, CandSet cands) {
    grid->cands[cell] &= ~cands;
}

void grid_cell_clear_cands(Grid *grid, int cell) {
    cand_set_clear(&grid->cands[cell]);
}

int grid_cell_first_cand(Grid *grid, int cell) {
    return cand_set_first(grid->cands[cell]);
}

CandSet grid_region_cands_intersection(Grid *grid, int region[],
                                       int region_len) {
    CandSet *sets = malloc(region_len * sizeof(CandSet));

    for (int i = 0; i < region_len; i++) {
        sets[i] = grid->cands[region[i]];
    }

    CandSet result = cand_set_intersection(sets, region_len);

    free(sets);

    return result;
}

CandSet grid_region_cands_union(Grid *grid, int region[], int region_len) {
    CandSet *sets = malloc(region_len * sizeof(CandSet));

    for (int i = 0; i < region_len; i++) {
        sets[i] = grid->cands[region[i]];
    }

    CandSet result = cand_set_union(sets, region_len);

    free(sets);

    return result;
}

int grid_region_missing_values(Grid *grid, int region[], int region_len,
                               int out[]) {
    CandSet missing_values = CAND_SET_FULL;
    for (int i = 0; i < region_len; i++) {
        cand_set_remove(&missing_values, grid_cell_value(grid, region[i]));
    }
    return cand_set_to_arr(missing_values, out);
}

int grid_region_with_cand(Grid *grid, int region[], int region_len, int cand,
                          int out[]) {
    int count = 0;
    for (int i = 0; i < region_len; i++) {
        if (grid_cell_has_cand(grid, region[i], cand)) {
            out[count++] = region[i];
        }
    }
    return count;
}

int grid_region_with_n_cands_max(Grid *grid, int region[], int region_len,
                                 int n, int out[]) {
    int count = 0;
    for (int i = 0; i < region_len; i++) {
        int num_cands = grid_cell_num_cands(grid, region[i]);
        if (num_cands > 0 && num_cands <= n) {
            out[count++] = region[i];
        }
    }
    return count;
}

int grid_region_with_cands_some(Grid *grid, int region[], int region_len,
                                CandSet cands, int out[]) {
    int count = 0;
    for (int i = 0; i < region_len; i++) {
        if (cand_set_len(grid->cands[region[i]] & cands) > 0) {
            out[count++] = region[i];
        }
    }
    return count;
}

static Grid *grid_from_values(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));
    grid->num_empty = 81;

    for (int i = 0; i < 81; i++) {
        grid->values[i] = 0;
        grid->cands[i] = CAND_SET_FULL;
        grid->clues[i] = false;
    }

    for (int i = 0; i < 81; i++) {
        int value = isdigit(grid_str[i]) ? grid_str[i] - '0' : 0;
        if (value != 0) {
            grid_fill_cell(grid, i, value);
            grid->clues[i] = true;
        }
    }

    return grid;
}

static Grid *grid_from_cands(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));
    grid->num_empty = 81;

    for (int i = 0; i < 81; i++) {
        char cell_str[] = {grid_str[i * 2], grid_str[i * 2 + 1], '\0'};
        int cell_bits = strtol(cell_str, NULL, 36);

        grid->values[i] = 0;
        grid->cands[i] = CAND_SET_EMPTY;
        grid->clues[i] = false;

        if (cell_bits <= 9) {
            grid->values[i] = cell_bits;
            grid->clues[i] = true;
            grid->num_empty--;
        } else if (cell_bits <= 18) {
            grid->values[i] = cell_bits - 9;
            grid->num_empty--;
        } else {
            grid->cands[i] = cell_bits - 18;
        }
    }

    return grid;
}
