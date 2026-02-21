#ifndef NAKED_SET_H
#define NAKED_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

bool naked_set(Grid *grid, Step *step);
bool naked_pair(Grid *grid, Step *step);
bool naked_triple(Grid *grid, Step *step);
bool naked_quad(Grid *grid, Step *step);

void naked_set_apply(Grid *grid, Step *step);
void naked_set_revert(Grid *grid, Step *step);
void naked_set_explain(Ui *ui, Step *step);
void naked_set_colorise(ColorPair colors[81][9], Step *step);

#endif
