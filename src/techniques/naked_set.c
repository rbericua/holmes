#include "techniques/naked_set.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "grid.h"
#include "step.h"
#include "techniques/combinations.h"

static bool naked_n_set_unit(Grid *grid, Cell *units[9][9], Step *step,
                             int size, UnitType unit_type);

bool naked_set(Grid *grid, Step *step) {
    bool pair_result = naked_pair(grid, step);
    if (pair_result) return true;

    bool triple_result = naked_triple(grid, step);
    if (triple_result) return true;

    bool quad_result = naked_quad(grid, step);
    if (quad_result) return true;

    return false;
}

bool naked_pair(Grid *grid, Step *step) {
    step->tech = TECH_NAKED_PAIR;
    step->as.naked_set.size = 2;

    bool row_result = naked_n_set_unit(grid, grid->rows, step, 2, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, step, 2, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, step, 2, UNIT_BOX);
    if (box_result) return true;

    return false;
}

bool naked_triple(Grid *grid, Step *step) {
    step->tech = TECH_NAKED_TRIPLE;
    step->as.naked_set.size = 3;

    bool row_result = naked_n_set_unit(grid, grid->rows, step, 3, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, step, 3, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, step, 3, UNIT_BOX);
    if (box_result) return true;

    return false;
}

bool naked_quad(Grid *grid, Step *step) {
    step->tech = TECH_NAKED_QUAD;
    step->as.naked_set.size = 4;

    bool row_result = naked_n_set_unit(grid, grid->rows, step, 4, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, step, 4, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, step, 4, UNIT_BOX);
    if (box_result) return true;

    return false;
}

static bool naked_n_set_unit(Grid *grid, Cell *units[9][9], Step *step,
                             int size, UnitType unit_type) {
    NakedSetStep *s = &step->as.naked_set;
    s->unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        Cell *possible_cells[9];
        int num_possible_cells = cells_with_n_cands_max(unit, 9, size,
                                                        possible_cells);

        if (num_possible_cells < size) continue;

        int num_combs;
        Cell ***combs = generate_combinations(possible_cells,
                                              num_possible_cells, size,
                                              sizeof(Cell *), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            Cell **comb = combs[comb_i];

            CandSet comb_cands = cells_cand_union(comb, size);
            if (comb_cands.len != size) continue;

            Cell *common_peers[MAX_COMMON_PEERS];
            int num_common_peers = grid_common_peers(grid, comb, size,
                                                     common_peers);

            Cell *removal_cells[MAX_NAKED_SET_REMOVALS];
            int num_removals = cells_with_removals(
                common_peers, num_common_peers, comb_cands, removal_cells,
                s->removed_cands);

            if (num_removals == 0) continue;

            cells_idxs(comb, size, s->idxs);
            s->cands = comb_cands;
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
