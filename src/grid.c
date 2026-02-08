#include "grid.h"

#include <stdbool.h>
#include <stdlib.h>

#include "cand_set.h"
#include "cell.h"

static CandSet grid_cell_initial_cands(Grid *grid, Cell *cell);
static void grid_generate_neighbours(Grid *grid);

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

    grid_generate_neighbours(grid);

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

    for (int i = 0; i < NUM_NEIGHBOURS; i++) {
        cell_remove_cand(grid->neighbours[cell_idx(cell)][i], value);
    }
}

static CandSet grid_cell_initial_cands(Grid *grid, Cell *cell) {
    CandSet row_missing_values = cells_missing_values_to_set(
        grid->rows[cell->row], 9);
    CandSet col_missing_values = cells_missing_values_to_set(
        grid->cols[cell->col], 9);
    CandSet box_missing_values = cells_missing_values_to_set(
        grid->boxes[cell->box], 9);

    return cand_set_intersection(3, row_missing_values, col_missing_values,
                                 box_missing_values);
}

static void grid_generate_neighbours(Grid *grid) {
    for (int i = 0; i < 81; i++) {
        int count = 0;
        for (int j = 0; j < 81; j++) {
            if (cell_is_neighbour(grid->cells[i], grid->cells[j])) {
                grid->neighbours[i][count++] = grid->cells[j];
            }
        }
    }
}
