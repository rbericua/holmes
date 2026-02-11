#ifndef NAKED_SET_H
#define NAKED_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

bool naked_set(Grid *grid, Step *step);
bool naked_pair(Grid *grid, Step *step);
bool naked_triple(Grid *grid, Step *step);
bool naked_quad(Grid *grid, Step *step);

#endif
