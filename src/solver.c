#include "solver.h"

#include "grid.h"
#include "step.h"
#include "techniques/techniques.h"

SolveStatus solver_next_step(Grid *grid, Step *step) {
    for (int i = 0; i < NUM_TECHNIQUES; i++) {
        if (grid_is_solved(grid)) return SOLVE_COMPLETE;
        if (techniques[i](grid, step)) return SOLVE_ONGOING;
    }

    return SOLVE_STUCK;
}

void solver_apply_step(Grid *grid, Step *step) {
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        int idx = step->as.naked_single.idx;
        int value = step->as.naked_single.value;

        grid_fill_cell(grid, grid->cells[idx], value);
    } break;
    default: break;
    }
}
