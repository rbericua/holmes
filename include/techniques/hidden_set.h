#ifndef HIDDEN_SET_H
#define HIDDEN_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

bool hidden_set(Grid *grid, Step *step);
bool hidden_pair(Grid *grid, Step *step);
bool hidden_triple(Grid *grid, Step *step);
bool hidden_quad(Grid *grid, Step *step);

#endif
