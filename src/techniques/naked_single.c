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

        Cell *removal_cells[MAX_NAKED_SINGLE_REMOVALS];
        int num_removals = cells_with_cand(grid->peers[i], NUM_PEERS, value,
                                           removal_cells);

        step->as.naked_single.idx = i;
        step->as.naked_single.value = value;
        cells_idxs(removal_cells, num_removals,
                   step->as.naked_single.removal_idxs);
        step->as.naked_single.num_removals = num_removals;

        return true;
    }

    return false;
}
