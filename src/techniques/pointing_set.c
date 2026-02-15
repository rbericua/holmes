#include "techniques/pointing_set.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"
#include "step.h"

static bool pointing_set_unit(Grid *grid, Cell *units[9][9], Step *step,
                              UnitType unit_type);
static void find_removal_unit(Cell *cells[], UnitType trigger_type,
                              UnitType *out_type, int *out_idx);

bool pointing_set(Grid *grid, Step *step) {
    step->tech = TECH_POINTING_SET;

    bool row_result = pointing_set_unit(grid, grid->rows, step, UNIT_ROW);
    if (row_result) return true;

    bool col_result = pointing_set_unit(grid, grid->cols, step, UNIT_COL);
    if (col_result) return true;

    bool box_result = pointing_set_unit(grid, grid->boxes, step, UNIT_BOX);
    if (box_result) return true;

    return false;
}

static bool pointing_set_unit(Grid *grid, Cell *units[9][9], Step *step,
                              UnitType unit_type) {
    PointingSetStep *s = &step->as.pointing_set;
    s->trigger_unit_type = unit_type;

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

            if (num_possible_cells != 2 && num_possible_cells != 3) continue;

            Cell *common_peers[MAX_COMMON_PEERS];
            int num_common_peers = grid_common_peers(
                grid, possible_cells, num_possible_cells, common_peers);

            Cell *removal_cells[6];
            int num_removals = cells_with_cand(common_peers, num_common_peers,
                                               value, removal_cells);

            if (num_removals == 0) continue;

            cells_idxs(possible_cells, num_possible_cells, s->idxs);
            s->size = num_possible_cells;
            s->value = value;
            cells_idxs(removal_cells, num_removals, s->removal_idxs);
            s->num_removals = num_removals;
            s->trigger_unit_idx = unit_i;
            find_removal_unit(possible_cells, unit_type, &s->removal_unit_type,
                              &s->removal_unit_idx);

            return true;
        }
    }

    return false;
}

static void find_removal_unit(Cell *cells[], UnitType trigger_type,
                              UnitType *out_type, int *out_idx) {
    if (trigger_type != UNIT_BOX) {
        *out_type = UNIT_BOX;
        *out_idx = cells[0]->box;
    } else if (cells[0]->row == cells[1]->row) {
        *out_type = UNIT_ROW;
        *out_idx = cells[0]->row;
    } else {
        *out_type = UNIT_COL;
        *out_idx = cells[0]->col;
    }
}
