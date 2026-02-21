#ifndef NAKED_SINGLE_H
#define NAKED_SINGLE_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

bool naked_single(Grid *grid, Step *step);

void naked_single_apply(Grid *grid, Step *step);
void naked_single_revert(Grid *grid, Step *step);
void naked_single_explain(Ui *ui, Step *step);
void naked_single_colorise(ColorPair colors[81][9], Step *step);

#endif
