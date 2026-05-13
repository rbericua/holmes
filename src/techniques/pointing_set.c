#include "techniques/pointing_set.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/explain.h"
#include "util/dynstr.h"

static bool pointing_set_unit(Grid *grid, Step *step, UnitType unit_type);
static void find_removal_unit(int set_cells[], UnitType trigger_type,
                              UnitType *out_type, int *out_idx);

bool pointing_set(Grid *grid, Step *step) {
    step->type = TECH_POINTING_SET;

    if (pointing_set_unit(grid, step, UNIT_ROW)) return true;
    if (pointing_set_unit(grid, step, UNIT_COL)) return true;
    if (pointing_set_unit(grid, step, UNIT_BOX)) return true;
    return false;
}

void pointing_set_apply(Grid *grid, Step *step) {
    PointingSetStep *s = &step->as.pointing_set;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->value);
    }
}

void pointing_set_revert(Grid *grid, Step *step) {
    PointingSetStep *s = &step->as.pointing_set;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void pointing_set_explain(DynStr *buf, Step *step) {
    PointingSetStep *s = &step->as.pointing_set;

    char *set_name = explain_set_name(s->size);
    char *from_unit_name = explain_unit_name(s->trigger_unit_type);
    char *to_unit_name = explain_unit_name(s->removal_unit_type);
    char *idxs_str = explain_cells(s->set_cells, s->size);

    ds_append(buf, "[Pointing %s (%s %d -> %s %d)] {%d} on %s:\n", set_name,
              from_unit_name, s->trigger_unit_idx + 1, to_unit_name,
              s->removal_unit_idx + 1, s->value, idxs_str);

    free(set_name);
    free(from_unit_name);
    free(to_unit_name);
    free(idxs_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->value);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void pointing_set_colorise(ColorPair colors[81][9], Step *step) {
    PointingSetStep *s = &step->as.pointing_set;

    for (int i = 0; i < s->size; i++) {
        colors[s->set_cells[i]][s->value - 1] = CP_TRIGGER;
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}

static bool pointing_set_unit(Grid *grid, Step *step, UnitType unit_type) {
    PointingSetStep *s = &step->as.pointing_set;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        int *unit = units[unit_type][unit_i];

        int missing_values[9];
        int num_missing_values = grid_region_missing_values(grid, unit, 9,
                                                            missing_values);

        for (int value_i = 0; value_i < num_missing_values; value_i++) {
            int value = missing_values[value_i];

            int possible_cells[9];
            int num_possible_cells = grid_region_with_cand(grid, unit, 9, value,
                                                           possible_cells);

            if (num_possible_cells != 2 && num_possible_cells != 3) continue;

            int common_peers[MAX_COMMON_PEERS];
            int num_common_peers = cells_common_peers(
                possible_cells, num_possible_cells, common_peers);

            s->num_removals = grid_region_with_cand(
                grid, common_peers, num_common_peers, value, s->removal_cells);

            if (s->num_removals == 0) continue;

            memcpy(s->set_cells, possible_cells,
                   num_possible_cells * sizeof(int));
            s->size = num_possible_cells;
            s->value = value;
            s->trigger_unit_type = unit_type;
            s->trigger_unit_idx = unit_i;
            find_removal_unit(s->set_cells, unit_type, &s->removal_unit_type,
                              &s->removal_unit_idx);

            return true;
        }
    }

    return false;
}

static void find_removal_unit(int set_cells[], UnitType trigger_type,
                              UnitType *out_type, int *out_idx) {
    if (trigger_type != UNIT_BOX) {
        *out_type = UNIT_BOX;
        *out_idx = cell_box(set_cells[0]);
    } else if (cell_row(set_cells[0]) == cell_row(set_cells[1])) {
        *out_type = UNIT_ROW;
        *out_idx = cell_row(set_cells[0]);
    } else {
        *out_type = UNIT_COL;
        *out_idx = cell_col(set_cells[0]);
    }
}
