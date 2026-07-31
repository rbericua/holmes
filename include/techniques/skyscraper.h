#ifndef SKYSCRAPER_H
#define SKYSCRAPER_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui/colors.h"
#include "util/dynstr.h"

bool skyscraper(Grid *grid, Step *step);

void skyscraper_apply(Grid *grid, Step *step);
void skyscraper_revert(Grid *grid, Step *step);
void skyscraper_explain(DynStr *buf, Step *step);
void skyscraper_colorise(ColorPair colors[81][9], Step *step);

#endif
