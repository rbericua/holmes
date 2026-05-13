#include "techniques/naked_set.h"

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

static bool naked_n_set(Grid *grid, Step *step, int size);
static int find_removals(Grid *grid, int cells[], int num_cells, CandSet cands,
                         int out_cells[], CandSet out_cands[]);
static void find_units(int set_cells[], int size, int removal_cells[],
                       int num_removals, int unit_idx, UnitType unit_type,
                       int units[3]);

bool naked_pair(Grid *grid, Step *step) {
    step->type = TECH_NAKED_PAIR;
    return naked_n_set(grid, step, 2);
}

bool naked_triple(Grid *grid, Step *step) {
    step->type = TECH_NAKED_TRIPLE;
    return naked_n_set(grid, step, 3);
}

bool naked_quad(Grid *grid, Step *step) {
    step->type = TECH_NAKED_QUAD;
    return naked_n_set(grid, step, 4);
}

void naked_set_apply(Grid *grid, Step *step) {
    NakedSetStep *s = &step->as.naked_set;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cands(grid, s->removal_cells[i], s->removal_cands[i]);
    }
}

void naked_set_revert(Grid *grid, Step *step) {
    NakedSetStep *s = &step->as.naked_set;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cands(grid, s->removal_cells[i], s->removal_cands[i]);
    }
}

void naked_set_explain(DynStr *buf, Step *step) {
    NakedSetStep *s = &step->as.naked_set;

    char *set_name = explain_set_name(s->size);
    char *units_str = explain_units(s->units);
    char *cands_str = explain_cand_set(s->set_cands);
    char *idxs_str = explain_cells(s->set_cells, s->size);

    ds_append(buf, "[Naked %s (%s)] %s on %s:\n", set_name, units_str,
              cands_str, idxs_str);

    free(set_name);
    free(units_str);
    free(cands_str);
    free(idxs_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_cands_removal(s->removal_cells[i],
                                                  s->removal_cands[i]);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void naked_set_colorise(ColorPair colors[81][9], Step *step) {
    NakedSetStep *s = &step->as.naked_set;

    for (int i = 0; i < s->size; i++) {
        for (int value = 1; value <= 9; value++) {
            if (cand_set_has(s->set_cands, value)) {
                colors[s->set_cells[i]][value - 1] = CP_TRIGGER;
            }
        }
    }
    for (int i = 0; i < s->num_removals; i++) {
        for (int value = 1; value <= 9; value++) {
            if (cand_set_has(s->removal_cands[i], value)) {
                colors[s->removal_cells[i]][value - 1] = CP_REMOVAL;
            }
        }
    }
}

static bool naked_n_set(Grid *grid, Step *step, int size) {
    NakedSetStep *s = &step->as.naked_set;

    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];

            int possible_cells[9];
            int num_possible_cells = grid_region_with_n_cands_max(
                grid, unit, 9, size, possible_cells);

            if (num_possible_cells < size) continue;

            int num_possible_sets;
            int **possible_sets = generate_combinations(
                possible_cells, num_possible_cells, size, sizeof(int),
                &num_possible_sets);

            for (int set_i = 0; set_i < num_possible_sets; set_i++) {
                int *set = possible_sets[set_i];

                CandSet set_cands = grid_region_cands_union(grid, set, size);

                if (cand_set_len(set_cands) != size) continue;

                int common_peers[MAX_COMMON_PEERS];
                int num_common_peers = cells_common_peers(set, size,
                                                          common_peers);

                s->num_removals = find_removals(
                    grid, common_peers, num_common_peers, set_cands,
                    s->removal_cells, s->removal_cands);

                if (s->num_removals == 0) continue;

                memcpy(s->set_cells, set, size * sizeof(int));
                s->size = size;
                s->set_cands = set_cands;
                find_units(set, size, s->removal_cells, s->num_removals, unit_i,
                           unit_type, s->units);

                free_combinations(possible_sets);

                return true;
            }

            free_combinations(possible_sets);
        }
    }

    return false;
}

static int find_removals(Grid *grid, int cells[], int num_cells, CandSet cands,
                         int out_cells[], CandSet out_cands[]) {
    int count = 0;
    for (int i = 0; i < num_cells; i++) {
        CandSet common_cands = grid_cell_cands(grid, cells[i]) & cands;

        if (cand_set_len(common_cands) > 0) {
            out_cells[count] = cells[i];
            out_cands[count++] = common_cands;
        }
    }
    return count;
}

static void find_units(int set_cells[], int size, int removal_cells[],
                       int num_removals, int unit_idx, UnitType unit_type,
                       int units[3]) {
    units[UNIT_ROW] = unit_type == UNIT_ROW ? unit_idx : -1;
    units[UNIT_COL] = unit_type == UNIT_COL ? unit_idx : -1;
    units[UNIT_BOX] = unit_type == UNIT_BOX
                              || cells_in_same_box(set_cells, size)
                          ? cell_box(set_cells[0])
                          : -1;

    bool removals_in_unit[3] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < num_removals; j++) {
            if (cell_unit(removal_cells[j], i) == units[i]) {
                removals_in_unit[i] = true;
                break;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        if (!removals_in_unit[i]) {
            units[i] = -1;
        }
    }
}
