#ifndef TECHNIQUES_H
#define TECHNIQUES_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"

#include "techniques/hidden_single.h"
#include "techniques/naked_single.h"

typedef bool (*TechniqueFn)(Grid *, Step *);

TechniqueFn techniques[] = {naked_single, hidden_single};

#endif
