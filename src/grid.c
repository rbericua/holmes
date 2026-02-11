#include "grid.h"

#include <stdbool.h>
#include <stdlib.h>

#include "cand_set.h"
#include "cell.h"

static CandSet grid_cell_initial_cands(Grid *grid, Cell *cell);
static void grid_generate_peers(Grid *grid);

Grid *grid_create(char *grid_str) {
    Grid *grid = malloc(sizeof(Grid));

    grid->empty_cells = 81;

    for (int i = 0; i < 81; i++) {
        int value = grid_str[i] - '0';
        Cell *cell = cell_create(i, value);

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
