#include "solver.h"

#include "cell.h"
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
        NakedSingleStep *s = &step->as.naked_single;

        grid_fill_cell(grid, grid->cells[s->idx], s->value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *s = &step->as.hidden_single;

        grid_fill_cell(grid, grid->cells[s->idx], s->value);
    } break;
    case TECH_NAKED_PAIR:
    case TECH_NAKED_TRIPLE:
    case TECH_NAKED_QUAD: {
        NakedSetStep *s = &step->as.naked_set;

        for (int i = 0; i < s->num_removals; i++) {
            cell_remove_cands(grid->cells[s->removal_idxs[i]], s->cands);
        }
    } break;
    case TECH_HIDDEN_PAIR:
    case TECH_HIDDEN_TRIPLE:
    case TECH_HIDDEN_QUAD: {
        HiddenSetStep *s = &step->as.hidden_set;

        for (int i = 0; i < s->size; i++) {
            grid->cells[s->idxs[i]]->cands = s->cands;
        }
    } break;
    case TECH_POINTING_SET: {
        PointingSetStep *s = &step->as.pointing_set;

        for (int i = 0; i < s->num_removals; i++) {
            cell_remove_cand(grid->cells[s->removal_idxs[i]], s->value);
        }
    } break;
    default: break;
    }
}
