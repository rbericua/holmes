#include "history.h"

#include "dynarr.h"
#include "grid.h"
#include "solver.h"
#include "step.h"

bool history_can_undo(History *hist) {
    return hist->curr > 0;
}

bool history_can_redo(History *hist) {
    return hist->curr < hist->steps.len;
}

bool history_undo(History *hist, Grid *grid) {
    if (!history_can_undo(hist)) return false;
    hist->curr--;
    solver_revert_step(grid, history_curr(hist));
    return true;
}

bool history_redo(History *hist, Grid *grid) {
    if (!history_can_redo(hist)) return false;
    solver_apply_step(grid, history_curr(hist));
    hist->curr++;
    return true;
}

Step *history_curr(History *hist) {
    if (hist->curr == 0) return NULL;
    return &hist->steps.elems[hist->curr - 1];
}

void history_add(History *hist, Step step) {
    da_append(&hist->steps, step);
    hist->curr++;
}

void history_free(History *hist) {
    da_deinit(&hist->steps);
}
