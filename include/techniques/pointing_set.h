#ifndef POINTING_SET_H
#define POINTING_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

bool pointing_set(Grid *grid, Step *out_step);

#endif
