#ifndef BASIC_FISH_H
#define BASIC_FISH_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

bool x_wing(Grid *grid, Step *step);
bool swordfish(Grid *grid, Step *step);
bool jellyfish(Grid *grid, Step *step);

#endif
