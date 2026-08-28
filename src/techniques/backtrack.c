#include "techniques/backtrack.h"

#include <stdbool.h>

#include "cand_set.h"
#include "geometry.h"
#include "grid.h"

CandSet rows[9];
CandSet cols[9];
CandSet boxes[9];

static void backtrack_rec(int values[], int empty_idxs[], int num_empty,
                          int *num_solutions);

static void place_value(int values[], int idx, int value);
static void remove_value(int values[], int idx, int value);
static bool is_placement_valid(int idx, int value);
static CandSet get_cell_cands(int idx);
static int find_min_cands(int empty_idxs[], int num_empty, int values[]);

int backtrack(Grid *grid) {
    for (int i = 0; i < 9; i++) {
        rows[i] = CAND_SET_EMPTY;
        cols[i] = CAND_SET_EMPTY;
        boxes[i] = CAND_SET_EMPTY;
    }

    int values[81] = {0};
    int empty_idxs[81];
    int num_empty = 0;

    for (int i = 0; i < 81; i++) {
        if (grid_cell_is_empty(grid, i)) {
            empty_idxs[num_empty++] = i;
        } else {
            int value = grid_cell_value(grid, i);
            if (!is_placement_valid(i, value)) return 0;
            place_value(values, i, value);
        }
    }

    int num_solutions = 0;
    backtrack_rec(values, empty_idxs, num_empty, &num_solutions);
    return num_solutions;
}

static void backtrack_rec(int values[], int empty_idxs[], int num_empty,
                          int *num_solutions) {
    int curr = find_min_cands(empty_idxs, num_empty, values);
    if (curr == -1) {
        (*num_solutions)++;
        return;
    }

    CandSet cands = get_cell_cands(curr);
    while (!cand_set_is_empty(cands)) {
        int value = cand_set_pop(&cands);
        values[curr] = value;
        place_value(values, curr, value);
        backtrack_rec(values, empty_idxs, num_empty, num_solutions);
        if (*num_solutions > 1) return;
        remove_value(values, curr, value);
    }
}

static void place_value(int values[], int idx, int value) {
    values[idx] = value;
    rows[cell_row(idx)] = cand_set_add(rows[cell_row(idx)], value);
    cols[cell_col(idx)] = cand_set_add(cols[cell_col(idx)], value);
    boxes[cell_box(idx)] = cand_set_add(boxes[cell_box(idx)], value);
}

static void remove_value(int values[], int idx, int value) {
    values[idx] = 0;
    rows[cell_row(idx)] = cand_set_remove(rows[cell_row(idx)], value);
    cols[cell_col(idx)] = cand_set_remove(cols[cell_col(idx)], value);
    boxes[cell_box(idx)] = cand_set_remove(boxes[cell_box(idx)], value);
}

static bool is_placement_valid(int idx, int value) {
    return cand_set_has(get_cell_cands(idx), value);
}

static CandSet get_cell_cands(int idx) {
    int row = cell_row(idx);
    int col = cell_col(idx);
    int box = cell_box(idx);
    return CAND_SET_FULL & ~(rows[row] | cols[col] | boxes[box]);
}

static int find_min_cands(int empty_idxs[], int num_empty, int values[]) {
    int min_idx = -1;
    int min_cands = -1;
    for (int i = 1; i < num_empty; i++) {
        int idx = empty_idxs[i];
        if (values[idx] != 0) continue;
        int num_cands = cand_set_len(get_cell_cands(idx));
        if (min_cands == -1 || num_cands < min_cands) {
            min_idx = idx;
        }
    }
    return min_idx;
}
