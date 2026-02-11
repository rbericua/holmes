#include "techniques/naked_set.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "grid.h"
#include "step.h"
#include "techniques/combinations.h"

static bool naked_n_set_unit(Grid *grid, Cell *units[9][9], Step *out_step,
                             int n, UnitType unit_type);

bool naked_set(Grid *grid, Step *out_step) {
    out_step->tech = TECH_NAKED_SET;

    bool pair_result = naked_pair(grid, out_step);
    if (pair_result) return true;

    bool triple_result = naked_triple(grid, out_step);
    if (triple_result) return true;

    bool quad_result = naked_quad(grid, out_step);
    if (quad_result) return true;

    return false;
}

bool naked_pair(Grid *grid, Step *out_step) {
    out_step->as.naked_set.set_size = 2;

    bool row_result = naked_n_set_unit(grid, grid->rows, out_step, 2, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, out_step, 2, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, out_step, 2,
                                       UNIT_BOX);
    if (box_result) return true;

    return false;
}

bool naked_triple(Grid *grid, Step *out_step) {
    out_step->as.naked_set.set_size = 3;

    bool row_result = naked_n_set_unit(grid, grid->rows, out_step, 3, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, out_step, 3, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, out_step, 3,
                                       UNIT_BOX);
    if (box_result) return true;

    return false;
}

bool naked_quad(Grid *grid, Step *out_step) {
    out_step->as.naked_set.set_size = 4;

    bool row_result = naked_n_set_unit(grid, grid->rows, out_step, 4, UNIT_ROW);
    if (row_result) return true;

    bool col_result = naked_n_set_unit(grid, grid->cols, out_step, 4, UNIT_COL);
    if (col_result) return true;

    bool box_result = naked_n_set_unit(grid, grid->boxes, out_step, 4,
                                       UNIT_BOX);
    if (box_result) return true;

    return false;
}

static bool naked_n_set_unit(Grid *grid, Cell *units[9][9], Step *out_step,
                             int set_size, UnitType unit_type) {
    out_step->as.naked_set.unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        Cell *possible_cells[9];
        int num_possible_cells = cells_with_n_cands_max(unit, 9, set_size,
                                                        possible_cells);

        if (num_possible_cells < set_size) continue;

        int num_combs;
        Cell ***combs = generate_combinations(possible_cells,
                                              num_possible_cells, set_size,
                                              sizeof(Cell *), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            Cell **comb = combs[comb_i];

            CandSet comb_cands = cells_cand_union(comb, set_size);
            if (comb_cands.len != set_size) continue;

            Cell *common_peers[MAX_COMMON_PEERS];
            int num_common_peers = grid_common_peers(grid, comb, set_size,
                                                     common_peers);

            Cell *removal_cells[MAX_COMMON_PEERS];
            int num_removals = cells_with_removals(
                common_peers, num_common_peers, comb_cands, removal_cells,
                out_step->as.naked_set.removed_cands);

            if (num_removals == 0) continue;

            cells_idxs(comb, set_size, out_step->as.naked_set.set_idxs);
            out_step->as.naked_set.set_cands = comb_cands;
            cells_idxs(removal_cells, num_removals,
                       out_step->as.naked_set.removal_idxs);
            out_step->as.naked_set.num_removals = num_removals;
            out_step->as.naked_set.unit_idx = unit_i;

            free_combinations(combs);

            return true;
        }

        free_combinations(combs);
    }

    return false;
}
