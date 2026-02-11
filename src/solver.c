#include "solver.h"

#include "cell.h"
#include "grid.h"
#include "step.h"
#include "techniques/techniques.h"

SolveStatus solver_next_step(Grid *grid, Step *out_step) {
    for (int i = 0; i < NUM_TECHNIQUES; i++) {
        if (grid_is_solved(grid)) return SOLVE_COMPLETE;
        if (techniques[i](grid, out_step)) return SOLVE_ONGOING;
    }

    return SOLVE_STUCK;
}

void solver_apply_step(Grid *grid, Step *step) {
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        NakedSingleStep *naked_single = &step->as.naked_single;

        grid_fill_cell(grid, grid->cells[naked_single->idx],
                       naked_single->value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *hidden_single = &step->as.hidden_single;

        grid_fill_cell(grid, grid->cells[hidden_single->idx],
                       hidden_single->value);
    } break;
    case TECH_NAKED_SET: {
        NakedSetStep *naked_set = &step->as.naked_set;

        for (int i = 0; i < naked_set->num_removals; i++) {
            cell_remove_cands(grid->cells[naked_set->removal_idxs[i]],
                              naked_set->set_cands);
        }
    } break;
    default: break;
    }
}
