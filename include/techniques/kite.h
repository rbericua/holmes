#ifndef KITE_H
#define KITE_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui/colors.h"
#include "util/dynstr.h"

bool kite(Grid *grid, Step *step);

void kite_apply(Grid *grid, Step *step);
void kite_revert(Grid *grid, Step *step);
void kite_explain(DynStr *buf, Step *step);
void kite_colorise(ColorPair colors[81][9], Step *step);

#endif
