#include "solver.h"

#include "grid.h"
#include "step.h"
#include "techniques/registry.h"

SolveStatus solver_next_step(Grid *grid, Step *step) {
    for (int i = 0; i < NUM_TECHNIQUES; i++) {
        if (grid_is_solved(grid)) return SOLVE_COMPLETE;
        if (techniques[i](grid, step)) return SOLVE_ONGOING;
    }

    return SOLVE_STUCK;
}

void solver_apply_step(Grid *grid, Step *step) {
    if (!step) return;
    technique_ops[step->tech].apply(grid, step);
}

void solver_revert_step(Grid *grid, Step *step) {
    if (!step) return;
    technique_ops[step->tech].revert(grid, step);
}
