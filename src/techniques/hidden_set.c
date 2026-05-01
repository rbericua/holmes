#include "techniques/hidden_set.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/explain.h"
#include "util/combinations.h"
#include "util/dynstr.h"

static bool hidden_n_set(Grid *grid, Step *step, int set_size);
static int find_removals(Grid *grid, int cells[], int num_cells, CandSet cands,
                         CandSet out[]);
static void find_units(int cells[], int num_cells, int unit_idx, int unit_type,
                       int units[3]);

bool hidden_pair(Grid *grid, Step *step) {
    step->type = TECH_HIDDEN_PAIR;
    return hidden_n_set(grid, step, 2);
}

bool hidden_triple(Grid *grid, Step *step) {
    step->type = TECH_HIDDEN_TRIPLE;
    return hidden_n_set(grid, step, 3);
}

bool hidden_quad(Grid *grid, Step *step) {
    step->type = TECH_HIDDEN_QUAD;
    return hidden_n_set(grid, step, 4);
}

void hidden_set_apply(Grid *grid, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->set_size; i++) {
        grid_cell_remove_cands(grid, s->set_cells[i], s->removal_cands[i]);
    }
}

void hidden_set_revert(Grid *grid, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->set_size; i++) {
        grid_cell_add_cands(grid, s->set_cells[i], s->removal_cands[i]);
    }
}

void hidden_set_explain(DynStr *buf, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    char *set_name = explain_set_name(s->set_size);
    char *units_str = explain_units(s->units);
    char *cands_str = explain_cand_set(s->set_cands);
    char *idxs_str = explain_cells(s->set_cells, s->set_size);

    ds_append(buf, "[Hidden %s (%s)] %s on %s:\n", set_name, units_str,
              cands_str, idxs_str);

    free(set_name);
    free(units_str);
    free(cands_str);
    free(idxs_str);

    for (int i = 0; i < s->set_size; i++) {
        if (cand_set_len(s->removal_cands[i]) == 0) continue;

        char *removal_msg = explain_cands_removal(s->set_cells[i],
                                                  s->removal_cands[i]);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void hidden_set_colorise(ColorPair colors[81][9], Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->set_size; i++) {
        for (int cand = 1; cand <= 9; cand++) {
            colors[s->set_cells[i]][cand - 1] = cand_set_has(s->set_cands, cand)
                                                    ? CP_TRIGGER
                                                    : CP_REMOVAL;
        }
    }
}

static bool hidden_n_set(Grid *grid, Step *step, int set_size) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];

            int missing_values[9];
            int num_missing_values = grid_region_missing_values(grid, unit, 9,
                                                                missing_values);

            int num_combs;
            int **combs = generate_combinations(missing_values,
                                                num_missing_values, set_size,
                                                sizeof(int), &num_combs);

            for (int comb_i = 0; comb_i < num_combs; comb_i++) {
                CandSet cands = cand_set_from_arr(combs[comb_i], set_size);

                int possible_set[9];
                int possible_set_size = grid_region_with_cands_some(
                    grid, unit, 9, cands, possible_set);

                if (possible_set_size != set_size) continue;

                int num_removals = find_removals(grid, possible_set, set_size,
                                                 cands, s->removal_cands);

                if (num_removals == 0) continue;

                memcpy(s->set_cells, possible_set, set_size * sizeof(int));
                s->set_size = set_size;
                s->set_cands = cands;
                find_units(s->set_cells, set_size, unit_i, unit_type, s->units);

                free_combinations(combs);

                return true;
            }

            free_combinations(combs);
        }
    }

    return false;
}

static int find_removals(Grid *grid, int cells[], int num_cells, CandSet cands,
                         CandSet out[]) {
    int num_removals = 0;
    for (int i = 0; i < num_cells; i++) {
        CandSet removal_cands = grid_cell_cands(grid, cells[i]) & ~cands;
        out[i] = removal_cands;
        if (cand_set_len(removal_cands) > 0) {
            num_removals++;
        }
    }
    return num_removals;
}

static void find_units(int cells[], int num_cells, int unit_idx, int unit_type,
                       int units[3]) {
    units[UNIT_ROW] = unit_type == UNIT_ROW ? unit_idx : -1;
    units[UNIT_COL] = unit_type == UNIT_COL ? unit_idx : -1;
    units[UNIT_BOX] = unit_type == UNIT_BOX
                              || cells_in_same_box(cells, num_cells)
                          ? cell_box(cells[0])
                          : -1;
}
