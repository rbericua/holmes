#ifndef BASIC_FISH_H
#define BASIC_FISH_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

bool x_wing(Grid *grid, Step *step);
bool swordfish(Grid *grid, Step *step);
bool jellyfish(Grid *grid, Step *step);

void basic_fish_apply(Grid *grid, Step *step);
void basic_fish_revert(Grid *grid, Step *step);
void basic_fish_explain(Ui *ui, Step *step);
void basic_fish_colorise(ColorPair colors[81][9], Step *step);

#endif
