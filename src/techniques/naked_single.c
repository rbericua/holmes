#include "techniques/naked_single.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "grid.h"
#include "step.h"
#include "ui.h"

bool naked_single(Grid *grid, Step *step) {
    step->tech = TECH_NAKED_SINGLE;

    for (int i = 0; i < 81; i++) {
        Cell *cell = grid->cells[i];

        if (cell->cands.len != 1) continue;

        int value = cell_only_cand(cell);

        Cell *removal_cells[MAX_NAKED_SINGLE_REMOVALS];
        int num_removals = cells_with_cand(grid->peers[i], NUM_PEERS, value,
                                           removal_cells);

        step->as.naked_single.idx = i;
        step->as.naked_single.value = value;
        cells_idxs(removal_cells, num_removals,
                   step->as.naked_single.removal_idxs);
        step->as.naked_single.num_removals = num_removals;

        return true;
    }

    return false;
}

void naked_single_apply(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    grid_fill_cell(grid, grid->cells[s->idx], s->value);
}

void naked_single_revert(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    grid->cells[s->idx]->value = 0;
    grid->cells[s->idx]->cands = cand_set_from_values(1, s->value);
    grid->empty_cells++;

    for (int i = 0; i < s->num_removals; i++) {
        cell_add_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void naked_single_explain(Ui *ui, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    int row = ROW_FROM_IDX(s->idx);
    int col = COL_FROM_IDX(s->idx);

    ui_print_message(ui, false, false, "[Naked Single] Set r%dc%d to %d\n",
                     row + 1, col + 1, s->value);
}

void naked_single_colorise(ColorPair colors[81][9], Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    colors[s->idx][s->value - 1] = CP_TRIGGER;
    for (int i = 0; i < s->num_removals; i++) {
        int removal_idx = s->removal_idxs[i];
        colors[removal_idx][s->value - 1] = CP_REMOVAL;
    }
}
