#ifndef POINTING_SET_H
#define POINTING_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

bool pointing_set(Grid *grid, Step *step);

void pointing_set_apply(Grid *grid, Step *step);
void pointing_set_revert(Grid *grid, Step *step);
void pointing_set_explain(Ui *ui, Step *step);
void pointing_set_colorise(ColorPair colors[81][9], Step *step);

#endif
