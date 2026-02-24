#ifndef HIDDEN_SINGLE_H
#define HIDDEN_SINGLE_H

#include <stdbool.h>

#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "ui.h"

bool hidden_single(Grid *grid, Step *step);

void hidden_single_apply(Grid *grid, Step *step);
void hidden_single_revert(Grid *grid, Step *step);
void hidden_single_explain(DynStr *ds, Step *step);
void hidden_single_colorise(ColorPair colors[81][9], Step *step);

#endif
