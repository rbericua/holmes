#ifndef MEDUSA_H
#define MEDUSA_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui/colors.h"
#include "ui/pipes.h"
#include "util/dynstr.h"

bool medusa(Grid *grid, Step *step);

void medusa_apply(Grid *grid, Step *step);
void medusa_revert(Grid *grid, Step *step);
void medusa_explain(DynStr *buf, Step *step);
void medusa_colorise(ColorPair colors[81][9], Step *step);
void medusa_pipes(Pipes *pipes, Step *step);

#endif
