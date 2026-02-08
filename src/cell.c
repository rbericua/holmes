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

bool cell_is_neighbour(Cell *a, Cell *b) {
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

void cell_clear_cands(Cell *cell) {
    cand_set_clear(&cell->cands);
}

int cell_only_cand(Cell *cell) {
    return cand_set_only(cell->cands);
}

CandSet cells_missing_values(Cell *cells[], int num_cells) {
    CandSet missing_values = cand_set_full();
    for (int i = 0; i < num_cells; i++) {
        cand_set_remove(&missing_values, cells[i]->value);
    }
    return missing_values;
}
