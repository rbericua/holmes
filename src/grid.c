#include "grid.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "geometry.h"

static CandSet grid_cell_init_cands(Grid *grid, int cell);

static Grid *grid_from_values(char *grid_str);
static Grid *grid_from_sudoku_wiki(char *grid_str);
static Grid *grid_from_hodoku(char *grid_str);

Grid *grid_create(char *grid_str) {
    Grid *grid;

    if (strncmp(grid_str, "S9B", 3) == 0) {
        grid = grid_from_sudoku_wiki(&grid_str[3]);
    } else if (grid_str[0] == ':') {
        grid = grid_from_hodoku(&grid_str[1]);
    } else {
        grid = grid_from_values(grid_str);
    }

    return grid;
}

void grid_destroy(Grid *grid) {
    free(grid);
}

int grid_num_empty(Grid *grid) {
    return grid->num_empty;
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

CandSet grid_region_missing_values_to_set(Grid *grid, int region[],
                                          int region_len) {
    CandSet missing_values = CAND_SET_FULL;
    for (int i = 0; i < region_len; i++) {
        cand_set_remove(&missing_values, grid_cell_value(grid, region[i]));
    }
    return missing_values;
}

int grid_region_missing_values_to_arr(Grid *grid, int region[], int region_len,
                                      int out[]) {
    return cand_set_to_arr(
        grid_region_missing_values_to_set(grid, region, region_len), out);
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

static CandSet grid_cell_init_cands(Grid *grid, int cell) {
    CandSet init_cands = CAND_SET_FULL;
    for (int i = 0; i < 3; i++) {
        init_cands &= grid_region_missing_values_to_set(
            grid, units[i][cell_unit(cell, i)], 9);
    }
    return init_cands;
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

// See https://www.sudokuwiki.org/Sudoku_String_Definitions
static Grid *grid_from_sudoku_wiki(char *grid_str) {
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

// See hodoku.sourceforge.net/en/libs.php
static Grid *grid_from_hodoku(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));
    grid->num_empty = 81;

    grid_str = strchr(grid_str, ':') + 1;
    grid_str = strchr(grid_str, ':') + 1;

    int i = 0;
    while (*grid_str != ':') {
        if (*grid_str == '.') {
            grid->values[i] = 0;
            grid->clues[i] = false;
        } else if (*grid_str == '+') {
            grid->values[i] = *(++grid_str) - '0';
            grid->clues[i] = false;
            grid->num_empty--;
        } else {
            grid->values[i] = *grid_str - '0';
            grid->clues[i] = true;
            grid->num_empty--;
        }
        grid_str++;
        i++;
    }
    grid_str++;

    for (int i = 0; i < 81; i++) {
        grid->cands[i] = grid_cell_is_empty(grid, i)
                             ? grid_cell_init_cands(grid, i)
                             : CAND_SET_EMPTY;
    }

    while (*grid_str != ':') {
        if (*grid_str == ' ') {
            grid_str++;
        }

        int cand = *grid_str - '0';
        int row = *(grid_str + 1) - '0' - 1;
        int col = *(grid_str + 2) - '0' - 1;
        grid_cell_remove_cand(grid, cell_from_row_col(row, col), cand);

        grid_str += 3;
    }

    return grid;
}
