#include "grid.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "cell.h"

static Grid *grid_from_values(char *grid_str);
static Grid *grid_from_cands(char *grid_str);
static CandSet grid_cell_initial_cands(Grid *grid, Cell *cell);
static void grid_generate_peers(Grid *grid);

Grid *grid_create(char *grid_str) {
    Grid *grid;

    if (strncmp(grid_str, "S9B", 3) == 0) {
        grid = grid_from_cands(grid_str + 3);
    } else {
        grid = grid_from_values(grid_str);
    }

    grid_generate_peers(grid);

    return grid;
}

void grid_destroy(Grid *grid) {
    for (int i = 0; i < 81; i++) {
        cell_destroy(grid->cells[i]);
    }
    free(grid);
}

bool grid_is_solved(Grid *grid) {
    return grid->empty_cells == 0;
}

void grid_fill_cell(Grid *grid, Cell *cell, int value) {
    cell->value = value;
    cell_clear_cands(cell);
    grid->empty_cells--;

    for (int i = 0; i < NUM_PEERS; i++) {
        cell_remove_cand(grid->peers[cell_idx(cell)][i], value);
    }
}

int grid_common_peers(Grid *grid, Cell *cells[], int num_cells, Cell *out[]) {
    int count = 0;
    Cell **peers = grid->peers[cell_idx(cells[0])];
    for (int i = 0; i < NUM_PEERS; i++) {
        bool is_common = true;
        Cell *peer = peers[i];
        for (int j = 1; j < num_cells; j++) {
            if (!cell_is_peer(peer, cells[j])) {
                is_common = false;
                break;
            }
        }
        if (is_common) {
            out[count++] = peer;
        }
    }
    return count;
}

static Grid *grid_from_values(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));

    grid->empty_cells = 81;

    for (int i = 0; i < 81; i++) {
        char c = grid_str[i];
        int value = c >= '1' && c <= '9' ? c - '0' : 0;

        Cell *cell = cell_create(i, value, cand_set_empty(), value != 0);

        grid->rows[cell->row][cell->col] = cell;
        grid->cols[cell->col][cell->row] = cell;
        grid->boxes[cell->box][BOX_POSITION_FROM_IDX(i)] = cell;

        if (!cell_is_empty(cell)) {
            grid->empty_cells--;
        }
    }

    for (int i = 0; i < 81; i++) {
        Cell *cell = grid->cells[i];
        if (cell_is_empty(cell)) {
            cell->cands = grid_cell_initial_cands(grid, cell);
        }
    }

    return grid;
}

// The encoding format is defined here:
// https://www.sudokuwiki.org/Sudoku_String_Definitions
static Grid *grid_from_cands(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));

    grid->empty_cells = 81;

    for (int i = 0; i < 81; i++) {
        char cell_str[3] = {grid_str[i * 2], grid_str[i * 2 + 1], '\0'};
        unsigned long cell_bits = strtoul(cell_str, NULL, 36);

        int value;
        CandSet cands;
        bool is_clue;

        if (cell_bits <= 9) {
            value = cell_bits;
            cands = cand_set_empty();
            is_clue = true;
        } else if (cell_bits <= 18) {
            value = cell_bits - 9;
            cands = cand_set_empty();
            is_clue = false;
        } else {
            value = 0;
            cands = cand_set_from_mask(cell_bits - 18);
            is_clue = false;
        }

        Cell *cell = cell_create(i, value, cands, is_clue);

        grid->rows[cell->row][cell->col] = cell;
        grid->cols[cell->col][cell->row] = cell;
        grid->boxes[cell->box][BOX_POSITION_FROM_IDX(i)] = cell;

        if (!cell_is_empty(cell)) {
            grid->empty_cells--;
        }
    }

    return grid;
}

static CandSet grid_cell_initial_cands(Grid *grid, Cell *cell) {
    CandSet row_missing_values = cells_missing_values_to_set(
        grid->rows[cell->row], 9);
    CandSet col_missing_values = cells_missing_values_to_set(
        grid->cols[cell->col], 9);
    CandSet box_missing_values = cells_missing_values_to_set(
        grid->boxes[cell->box], 9);

    return cand_set_intersection_from_va(
        3, row_missing_values, col_missing_values, box_missing_values);
}

static void grid_generate_peers(Grid *grid) {
    for (int i = 0; i < 81; i++) {
        int count = 0;
        for (int j = 0; j < 81; j++) {
            if (cell_is_peer(grid->cells[i], grid->cells[j])) {
                grid->peers[i][count++] = grid->cells[j];
            }
        }
    }
}
