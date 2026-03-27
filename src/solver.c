#include "solver.h"

#include "grid.h"
#include "step.h"
#include "techniques/registry.h"

SolveStatus solver_next_step(Grid *grid, Step *step) {
    if (grid_is_solved(grid)) return SOLVE_COMPLETE;
    for (int i = 0; i < NUM_TECHNIQUES; i++) {
        if (techniques[i](grid, step)) return SOLVE_ONGOING;
    }
    return SOLVE_STUCK;
}

void solver_apply_step(Grid *grid, Step *step) {
    technique_ops[step->type].apply(grid, step);
}
