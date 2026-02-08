#include "techniques/naked_single.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"
#include "step.h"

bool naked_single(Grid *grid, Step *step) {
    step->tech = TECH_NAKED_SINGLE;

    for (int i = 0; i < 81; i++) {
        Cell *cell = grid->cells[i];

        if (cell->cands.len != 1) continue;

        int value = cell_only_cand(cell);

        step->as.naked_single.idx = i;
        step->as.naked_single.value = value;

        return true;
    }

    return false;
}
