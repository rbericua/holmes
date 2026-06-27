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
    s->len = 0;

    for (int i = 0; i < 81; i++) {
        if (grid_cell_num_cands(grid, i) != 1) continue;

        NakedSingleInfo *curr = &s->steps[s->len++];

        curr->cell = i;
        curr->value = grid_cell_first_cand(grid, i);
        curr->num_removals = grid_region_with_cand(
            grid, peers[i], NUM_PEERS, curr->value, curr->removal_cells);
    }

    return s->len > 0;
}

void naked_single_apply(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    for (int i = 0; i < s->len; i++) {
        grid_fill_cell(grid, s->steps[i].cell, s->steps[i].value);
    }
}

void naked_single_revert(Grid *grid, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    for (int i = 0; i < s->len; i++) {
        grid_clear_cell(grid, s->steps[i].cell,
                        cand_set_from_value(s->steps[i].value));

        for (int j = 0; j < s->steps[i].num_removals; j++) {
            grid_cell_add_cand(grid, s->steps[i].removal_cells[j],
                               s->steps[i].value);
        }
    }
}

void naked_single_explain(DynStr *buf, Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    ds_append(buf, "[Naked Single]\n");

    for (int i = 0; i < s->len; i++) {
        ds_append(buf, "- Set r%dc%d to %d\n", cell_row(s->steps[i].cell) + 1,
                  cell_col(s->steps[i].cell) + 1, s->steps[i].value);
    }
}

void naked_single_colorise(ColorPair colors[81][9], Step *step) {
    NakedSingleStep *s = &step->as.naked_single;

    for (int i = 0; i < s->len; i++) {
        colors[s->steps[i].cell][s->steps[i].value - 1] = CP_TRIGGER;
        for (int j = 0; j < s->steps[i].num_removals; j++) {
            colors[s->steps[i].removal_cells[j]][s->steps[i].value - 1] =
                CP_REMOVAL;
        }
    }
}
