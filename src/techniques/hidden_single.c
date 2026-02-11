#include "techniques/hidden_single.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"
#include "step.h"

static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *out_step,
                               UnitType unit_type);

bool hidden_single(Grid *grid, Step *out_step) {
    out_step->tech = TECH_HIDDEN_SINGLE;

    bool row_result = hidden_single_unit(grid, grid->rows, out_step, UNIT_ROW);
    if (row_result) return true;

    bool col_result = hidden_single_unit(grid, grid->cols, out_step, UNIT_COL);
    if (col_result) return true;

    bool box_result = hidden_single_unit(grid, grid->boxes, out_step, UNIT_BOX);
    if (box_result) return true;

    return false;
}

static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *out_step,
                               UnitType unit_type) {
    HiddenSingleStep *s = &out_step->as.hidden_single;
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

            s->idx = idx;
            s->value = value;
            s->unit_idx = unit_i;
            cells_idxs(grid->peers[idx], NUM_PEERS, s->peer_idxs);

            return true;
        }
    }

    return false;
}
