#include "techniques/backtrack.h"

#include <stdbool.h>

#include "geometry.h"
#include "grid.h"
#include "util/bits.h"

Bitset16 rows[9] = {0};
Bitset16 cols[9] = {0};
Bitset16 boxes[9] = {0};

static void place_value(int idx, int value);
static void remove_value(int idx, int value);
static bool is_placement_valid(int idx, int value);

int backtrack(Grid *grid) {
    int values[81];
    int empty_idxs[81];
    int num_empty = 0;

    for (int i = 0; i < 81; i++) {
        if (grid_cell_is_empty(grid, i)) {
            empty_idxs[num_empty++] = i;
        } else {
            values[i] = grid_cell_value(grid, i);
            place_value(i, values[i]);
        }
    }

    int num_solutions = 0;

    int empty_i = 0;
    while (true) {
        int curr = empty_idxs[empty_i];
        values[curr]++;

        if (values[curr] > 9) {
            if (empty_i == 0) break;

            values[curr] = 0;
            int prev = empty_idxs[--empty_i];
            remove_value(prev, values[prev]);
            continue;
        }

        if (!is_placement_valid(curr, values[curr])) continue;

        if (empty_i == num_empty - 1) {
            num_solutions++;
            if (num_solutions > 1) break;
            continue;
        }

        place_value(curr, values[curr]);
        empty_i++;
    }

    return num_solutions;
}

static void place_value(int idx, int value) {
    bitset16_add(&rows[cell_row(idx)], value);
    bitset16_add(&cols[cell_col(idx)], value);
    bitset16_add(&boxes[cell_box(idx)], value);
}

static void remove_value(int idx, int value) {
    bitset16_remove(&rows[cell_row(idx)], value);
    bitset16_remove(&cols[cell_col(idx)], value);
    bitset16_remove(&boxes[cell_box(idx)], value);
}

static bool is_placement_valid(int idx, int value) {
    int row = cell_row(idx);
    int col = cell_col(idx);
    int box = cell_box(idx);
    return !bitset16_has(rows[row], value) && !bitset16_has(cols[col], value)
           && !bitset16_has(boxes[box], value);
}
