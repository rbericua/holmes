#ifndef SIMPLE_COLORING_H
#define SIMPLE_COLORING_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"
#include "util/dynstr.h"

bool simple_coloring(Grid *grid, Step *step);

void simple_coloring_apply(Grid *grid, Step *step);
void simple_coloring_revert(Grid *grid, Step *step);
void simple_coloring_explain(DynStr *buf, Step *step);
void simple_coloring_colorise(ColorPair colors[81][9], Step *step);

#endif
