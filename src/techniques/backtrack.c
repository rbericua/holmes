#include "techniques/backtrack.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"

static int cells_with_value(Cell *cells[], int num_cells, int value);
static bool is_placement_valid(Grid *grid, int idx, int value);

int backtrack(Grid *grid) {
    int num_empty = 0;
    int empty_idxs[81];

    for (int i = 0; i < 81; i++) {
        if (cell_is_empty(grid->cells[i])) {
            empty_idxs[num_empty++] = i;
        }
    }

    int solutions = 0;

    int curr = 0;
    while (true) {
        int idx = empty_idxs[curr];
        Cell *cell = grid->cells[idx];

        if (cell->value == 9) {
            if (curr == 0) break;

            cell->value = 0;
            curr--;
            continue;
        }

        cell->value++;
        if (!is_placement_valid(grid, idx, cell->value)) continue;

        if (curr == num_empty - 1) {
            solutions++;
            if (solutions > 1) break;
            continue;
        }

        curr++;
    }

    for (int i = 0; i < num_empty; i++) {
        grid->cells[empty_idxs[i]]->value = 0;
    }

    return solutions;
}

static int cells_with_value(Cell *cells[], int num_cells, int value) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        if (cells[i]->value == value) {
            count++;
        }
    }
    return count;
}

static bool is_placement_valid(Grid *grid, int idx, int value) {
    Cell *cell = grid->cells[idx];

    if (cells_with_value(grid->rows[cell->row], 9, value) > 1) return false;
    if (cells_with_value(grid->cols[cell->col], 9, value) > 1) return false;
    if (cells_with_value(grid->boxes[cell->box], 9, value) > 1) return false;
    return true;
}
