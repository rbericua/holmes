#include "techniques/hidden_single.h"

#include <stdbool.h>
#include <stdlib.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/explain.h"
#include "util/dynstr.h"

static void find_units(int cell, int removal_cells[], int num_removals,
                       int units[3]);

bool hidden_single(Grid *grid, Step *step) {
    step->type = TECH_HIDDEN_SINGLE;
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];

            int missing_values[9];
            int num_missing_values = grid_region_missing_values(grid, unit, 9,
                                                                missing_values);

            for (int value_i = 0; value_i < num_missing_values; value_i++) {
                int value = missing_values[value_i];

                int possible_cells[9];
                int num_possible_cells = grid_region_with_cand(
                    grid, unit, 9, value, possible_cells);

                if (num_possible_cells != 1) continue;

                s->cell = possible_cells[0];
                s->value = value;
                s->old_cands = grid_cell_cands(grid, s->cell);
                s->num_removals = grid_region_with_cand(
                    grid, peers[s->cell], NUM_PEERS, value, s->removal_cells);
                find_units(s->cell, s->removal_cells, s->num_removals,
                           s->units);

                return true;
            }
        }
    }

    return false;
}

void hidden_single_apply(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    grid_fill_cell(grid, s->cell, s->value);
}

void hidden_single_revert(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    grid_clear_cell(grid, s->cell, s->old_cands);

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void hidden_single_explain(DynStr *buf, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    char *units_str = explain_units(s->units);

    ds_append(buf, "[Hidden Single (%s)] Set r%dc%d to %d\n", units_str,
              cell_row(s->cell) + 1, cell_col(s->cell) + 1, s->value);

    free(units_str);
}

void hidden_single_colorise(ColorPair colors[81][9], Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int cand = 1; cand <= 9; cand++) {
        colors[s->cell][cand - 1] = cand == s->value ? CP_TRIGGER : CP_REMOVAL;
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}

static void find_units(int cell, int removal_cells[], int num_removals,
                       int units[3]) {
    for (int i = 0; i < 3; i++) {
        units[i] = cell_unit(cell, i);
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < num_removals; j++) {
            if (cell_unit(removal_cells[j], i) == units[i]) {
                units[i] = -1;
                break;
            }
        }
    }
}
