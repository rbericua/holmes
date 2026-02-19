#include "techniques/hidden_single.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"
#include "step.h"
static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *step,
                               UnitType unit_type);

bool hidden_single(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_SINGLE;

    bool row_result = hidden_single_unit(grid, grid->rows, step, UNIT_ROW);
    if (row_result) return true;

    bool col_result = hidden_single_unit(grid, grid->cols, step, UNIT_COL);
    if (col_result) return true;

    bool box_result = hidden_single_unit(grid, grid->boxes, step, UNIT_BOX);
    if (box_result) return true;

    return false;
}

static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *step,
                               UnitType unit_type) {
    HiddenSingleStep *s = &step->as.hidden_single;
    s->unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        int missing_values[9];
        int num_missing_values = cells_missing_values_to_arr(unit, 9,
                                                             missing_values);

        for (int value_i = 0; value_i < num_missing_values; value_i++) {
            int value = missing_values[value_i];

            Cell *possible_cells[9];
            int num_possible_cells = cells_with_cand(unit, 9, value,
                                                     possible_cells);

            if (num_possible_cells != 1) continue;

            int idx = cell_idx(possible_cells[0]);

            Cell *removal_cells[MAX_HIDDEN_SINGLE_REMOVALS];
            int num_removals = cells_with_cand(grid->peers[idx], NUM_PEERS,
                                               value, removal_cells);

            s->idx = idx;
            s->value = value;
            cells_idxs(removal_cells, num_removals,
                       step->as.naked_single.removal_idxs);
            s->num_removals = num_removals;
            s->old_cands = possible_cells[0]->cands;
            s->unit_idx = unit_i;

            return true;
        }
    }

    return false;
}
