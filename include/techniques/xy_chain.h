#ifndef XY_CHAIN_H
#define XY_CHAIN_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui/colors.h"
#include "ui/pipes.h"
#include "util/dynstr.h"

bool xy_chain(Grid *grid, Step *step);

void xy_chain_apply(Grid *grid, Step *step);
void xy_chain_revert(Grid *grid, Step *step);
void xy_chain_explain(DynStr *buf, Step *step);
void xy_chain_colorise(ColorPair colors[81][9], Step *step);
void xy_chain_pipes(Pipes *pipes, Step *step);

#endif
