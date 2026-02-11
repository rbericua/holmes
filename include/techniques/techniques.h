#ifndef TECHNIQUES_H
#define TECHNIQUES_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

#include "techniques/hidden_set.h"
#include "techniques/hidden_single.h"
#include "techniques/naked_set.h"
#include "techniques/naked_single.h"
#include "techniques/pointing_set.h"

typedef bool (*TechniqueFn)(Grid *, Step *);

TechniqueFn techniques[] = {naked_single, hidden_single, naked_pair,
                            hidden_pair,  naked_triple,  hidden_triple,
                            naked_quad,   hidden_quad,   pointing_set};

#endif
