#include "techniques/hidden_set.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "grid.h"
#include "step.h"
#include "techniques/combinations.h"

static bool hidden_n_set_unit(Cell *units[9][9], Step *step, int size,
                              UnitType unit_type);

bool hidden_set(Grid *grid, Step *step) {
    if (hidden_pair(grid, step)) return true;
    if (hidden_triple(grid, step)) return true;
    if (hidden_quad(grid, step)) return true;
    return false;
}

bool hidden_pair(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_PAIR;
    step->as.hidden_set.size = 2;

    if (hidden_n_set_unit(grid->rows, step, 2, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 2, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 2, UNIT_BOX)) return true;
    return false;
}

bool hidden_triple(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_TRIPLE;
    step->as.hidden_set.size = 3;

    if (hidden_n_set_unit(grid->rows, step, 3, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 3, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 3, UNIT_BOX)) return true;
    return false;
}

bool hidden_quad(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_QUAD;
    step->as.hidden_set.size = 4;

    if (hidden_n_set_unit(grid->rows, step, 4, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 4, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 4, UNIT_BOX)) return true;
    return false;
}

static bool hidden_n_set_unit(Cell *units[9][9], Step *step, int size,
                              UnitType unit_type) {
    HiddenSetStep *s = &step->as.hidden_set;
    s->unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        int missing_values[9];
        int num_missing_values = cells_missing_values_to_arr(unit, 9,
                                                             missing_values);

        if (num_missing_values < size) continue;

        int num_combs;
        int **combs = generate_combinations(missing_values, num_missing_values,
                                            size, sizeof(int), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            CandSet comb_set = cand_set_from_arr(combs[comb_i], size);

            Cell *possible_cells[9];
            int num_possible_cells = cells_with_cands_some(unit, 9, comb_set,
                                                           possible_cells);

            if (num_possible_cells != size) continue;

            Cell *removal_cells[MAX_HIDDEN_SET_REMOVALS];
            int num_removals = cells_with_removals(
                possible_cells, num_possible_cells,
                cand_set_difference(cand_set_full(), comb_set), removal_cells,
                s->removed_cands);

            if (num_removals == 0) continue;

            cells_idxs(possible_cells, size, s->idxs);
            s->cands = comb_set;
            cells_idxs(removal_cells, num_removals, s->removal_idxs);
            s->num_removals = num_removals;
            s->unit_idx = unit_i;

            free_combinations(combs);

            return true;
        }

        free_combinations(combs);
    }

    return false;
}
