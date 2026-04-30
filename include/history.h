#ifndef HISTORY_H
#define HISTORY_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

typedef struct {
    Steps steps;
    int curr;
} History;

Step *history_curr(History *hist);
void history_save(History *hist, Step step);
bool history_can_undo(History *hist);
bool history_can_redo(History *hist);
bool history_undo(History *hist, Grid *grid);
bool history_redo(History *hist, Grid *grid);
void history_deinit(History *hist);

#endif
