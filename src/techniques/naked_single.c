#include "techniques/naked_single.h"

#include "cand_set.h"
#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "util/dynstr.h"

bool naked_single(Grid *grid, Step *step) {
    step->type = TECH_NAKED_SINGLE;
    NakedSingleStep *s = &step->as.naked_single;

    for (int i = 0; i < 81; i++) {
        if (grid_cell_num_cands(grid, i) != 1) continue;

        s->cell = i;
        s->value = grid_cell_first_cand(grid, i);
        s->num_removals = grid_region_with_cand(grid, peers[i], NUM_PEERS,
                                                s->value, s->removal_cells);

        return true;
    }

    return false;
}

void naked_single_apply(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    grid_fill_cell(grid, s->cell, s->value);
}

void naked_single_revert(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    grid_clear_cell(grid, s->cell, cand_set_from_value(s->value));

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void naked_single_explain(DynStr *buf, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    ds_append(buf, "[Naked Single] Set r%dc%d to %d\n", cell_row(s->cell) + 1,
              cell_col(s->cell) + 1, s->value);
}

void naked_single_colorise(ColorPair colors[81][9], Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    colors[s->cell][s->value - 1] = CP_TRIGGER;
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}
