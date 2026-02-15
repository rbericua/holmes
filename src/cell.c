#include "cell.h"

#include <stdbool.h>
#include <stdlib.h>

#include "cand_set.h"

Cell *cell_create(int idx, int value) {
    Cell *cell = malloc(sizeof(Cell));

    cell->value = value;
    cell->cands = cand_set_empty();
    cell->row = ROW_FROM_IDX(idx);
    cell->col = COL_FROM_IDX(idx);
    cell->box = BOX_FROM_IDX(idx);

    return cell;
}

void cell_destroy(Cell *cell) {
    free(cell);
}

int cell_idx(Cell *cell) {
    return IDX_FROM_ROW_COL(cell->row, cell->col);
}

bool cell_is_empty(Cell *cell) {
    return cell->value == 0;
}

bool cell_eq(Cell *a, Cell *b) {
    return cell_idx(a) == cell_idx(b);
}

bool cell_is_peer(Cell *a, Cell *b) {
    if (cell_eq(a, b)) return false;
    return a->row == b->row || a->col == b->col || a->box == b->box;
}

bool cell_has_cand(Cell *cell, int cand) {
    return cand_set_has(cell->cands, cand);
}

void cell_add_cand(Cell *cell, int cand) {
    cand_set_add(&cell->cands, cand);
}

void cell_remove_cand(Cell *cell, int cand) {
    cand_set_remove(&cell->cands, cand);
}
void cell_remove_cands(Cell *cell, CandSet cands) {
    cell->cands = cand_set_difference(cell->cands, cands);
}

void cell_clear_cands(Cell *cell) {
    cand_set_clear(&cell->cands);
}

int cell_only_cand(Cell *cell) {
    return cand_set_only(cell->cands);
}

void cells_idxs(Cell *cells[], int num_cells, int out[]) {
    for (int i = 0; i < num_cells; i++) {
        out[i] = cell_idx(cells[i]);
    }
}

CandSet cells_missing_values_to_set(Cell *cells[], int num_cells) {
    CandSet missing_values = cand_set_full();
    for (int i = 0; i < num_cells; i++) {
        cand_set_remove(&missing_values, cells[i]->value);
    }
    return missing_values;
}

int cells_missing_values_to_arr(Cell *cells[], int num_cells, int out[]) {
    return cand_set_to_arr(cells_missing_values_to_set(cells, num_cells), out);
}

CandSet cells_cand_intersection(Cell *cells[], int num_cells) {
    CandSet *sets = malloc(num_cells * sizeof(CandSet));

    for (int i = 0; i < num_cells; i++) {
        sets[i] = cells[i]->cands;
    }

    CandSet result = cand_set_intersection_from_arr(sets, num_cells);

    free(sets);

    return result;
}

CandSet cells_cand_union(Cell *cells[], int num_cells) {
    CandSet *sets = malloc(num_cells * sizeof(CandSet));

    for (int i = 0; i < num_cells; i++) {
        sets[i] = cells[i]->cands;
    }

    CandSet result = cand_set_union_from_arr(sets, num_cells);

    free(sets);

    return result;
}

int cells_with_cand(Cell *cells[], int num_cells, int cand, Cell *out[]) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        Cell *cell = cells[i];
        if (cell_has_cand(cell, cand)) {
            out[count++] = cell;
        }
    }
    return count;
}

int cells_with_cands_some(Cell *cells[], int num_cells, CandSet cands,
                          Cell *out[]) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        Cell *cell = cells[i];
        if (cand_set_intersection_from_va(2, cell->cands, cands).len != 0) {
            out[count++] = cell;
        }
    }
    return count;
}

int cells_with_n_cands_max(Cell *cells[], int num_cells, int n, Cell *out[]) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        Cell *cell = cells[i];
        if (cell->cands.len > 0 && cell->cands.len <= n) {
            out[count++] = cell;
        }
    }
    return count;
}

int cells_with_removals(Cell *cells[], int num_cells, CandSet cands,
                        Cell *out_cells[], CandSet out_cands[]) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        Cell *cell = cells[i];
        CandSet removed_cands = cand_set_intersection_from_va(2, cell->cands,
                                                              cands);
        if (removed_cands.len != 0) {
            out_cells[count] = cell;
            out_cands[count++] = removed_cands;
        }
    }
    return count;
}
