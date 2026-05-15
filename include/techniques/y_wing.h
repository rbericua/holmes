#ifndef Y_WING_H
#define Y_WING_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"
#include "util/dynstr.h"

bool y_wing(Grid *grid, Step *step);

void y_wing_apply(Grid *grid, Step *step);
void y_wing_revert(Grid *grid, Step *step);
void y_wing_explain(DynStr *buf, Step *step);
void y_wing_colorise(ColorPair colors[81][9], Step *step);

#endif
