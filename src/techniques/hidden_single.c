#include "techniques/hidden_single.h"

#include <stdbool.h>
#include <stdlib.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/explain.h"
#include "util/dynstr.h"

static HiddenSingleInfo *find_step(HiddenSingleStep *s, int cell) {
    for (int i = 0; i < s->len; i++) {
        if (s->steps[i].cell == cell) return &s->steps[i];
    }
    return NULL;
}

bool hidden_single(Grid *grid, Step *step) {
    step->type = TECH_HIDDEN_SINGLE;
    HiddenSingleStep *s = &step->as.hidden_single;
    s->len = 0;

    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];

            int missing_values[9];
            int num_missing_values = grid_region_missing_values_to_arr(
                grid, unit, 9, missing_values);

            for (int value_i = 0; value_i < num_missing_values; value_i++) {
                int value = missing_values[value_i];

                int possible_cells[9];
                int num_possible_cells = grid_region_with_cand(
                    grid, unit, 9, value, possible_cells);

                if (num_possible_cells != 1) continue;

                HiddenSingleInfo *curr = find_step(s, possible_cells[0]);
                if (!curr) {
                    curr = &s->steps[s->len++];
                    for (int i = 0; i < 3; i++) {
                        curr->units[i] = -1;
                    }
                }

                curr->cell = possible_cells[0];
                curr->value = value;
                curr->old_cands = grid_cell_cands(grid, curr->cell);
                curr->num_removals = grid_region_with_cand(
                    grid, peers[curr->cell], NUM_PEERS, value,
                    curr->removal_cells);
                curr->units[unit_type] = unit_i;
            }
        }
    }

    return s->len > 0;
}

void hidden_single_apply(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int i = 0; i < s->len; i++) {
        grid_fill_cell(grid, s->steps[i].cell, s->steps[i].value);
    }
}

void hidden_single_revert(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int i = 0; i < s->len; i++) {
        grid_clear_cell(grid, s->steps[i].cell, s->steps[i].old_cands);

        for (int j = 0; j < s->steps[i].num_removals; j++) {
            grid_cell_add_cand(grid, s->steps[i].removal_cells[j],
                               s->steps[i].value);
        }
    }
}

void hidden_single_explain(DynStr *buf, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    ds_append(buf, "[Hidden Single]\n");

    for (int i = 0; i < s->len; i++) {
        char *units_str = explain_units(s->steps[i].units);

        ds_append(buf, "- Set r%dc%d to %d (%s)\n",
                  cell_row(s->steps[i].cell) + 1,
                  cell_col(s->steps[i].cell) + 1, s->steps[i].value, units_str);

        free(units_str);
    }
}

void hidden_single_colorise(ColorPair colors[81][9], Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int i = 0; i < s->len; i++) {
        for (int cand = 1; cand <= 9; cand++) {
            colors[s->steps[i].cell][cand - 1] = cand == s->steps[i].value
                                                     ? CP_TRIGGER
                                                     : CP_REMOVAL;
        }
        for (int j = 0; j < s->steps[i].num_removals; j++) {
            colors[s->steps[i].removal_cells[j]][s->steps[i].value - 1] =
                CP_REMOVAL;
        }
    }
}
