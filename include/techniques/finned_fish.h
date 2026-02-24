#ifndef FINNED_FISH_H
#define FINNED_FISH_H

#include <stdbool.h>

#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "ui.h"

bool finned_x_wing(Grid *grid, Step *step);
bool finned_swordfish(Grid *grid, Step *step);
bool finned_jellyfish(Grid *grid, Step *step);

void finned_fish_apply(Grid *grid, Step *step);
void finned_fish_revert(Grid *grid, Step *step);
void finned_fish_explain(DynStr *ds, Step *step);
void finned_fish_colorise(ColorPair colors[81][9], Step *step);

#endif
