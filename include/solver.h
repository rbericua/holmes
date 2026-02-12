#ifndef SOLVER_H
#define SOLVER_H

#include "grid.h"
#include "step.h"

typedef enum {
    SOLVE_ONGOING,
    SOLVE_COMPLETE,
    SOLVE_STUCK
} SolveStatus;

SolveStatus solver_next_step(Grid *grid, Step *step);
void solver_apply_step(Grid *grid, Step *step);

#endif
