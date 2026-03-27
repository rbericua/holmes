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

int grid_cell_value(Grid *grid, int cell) {
    return grid->values[cell];
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

void grid_cell_remove_cand(Grid *grid, int cell, int cand) {
    cand_set_remove(&grid->cands[cell], cand);
}

void grid_cell_clear_cands(Grid *grid, int cell) {
    cand_set_clear(&grid->cands[cell]);
}

int grid_cell_first_cand(Grid *grid, int cell) {
    return cand_set_first(grid->cands[cell]);
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
