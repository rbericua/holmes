#ifndef HIDDEN_SET_H
#define HIDDEN_SET_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

bool hidden_set(Grid *grid, Step *step);
bool hidden_pair(Grid *grid, Step *step);
bool hidden_triple(Grid *grid, Step *step);
bool hidden_quad(Grid *grid, Step *step);

void hidden_set_apply(Grid *grid, Step *step);
void hidden_set_revert(Grid *grid, Step *step);
void hidden_set_explain(Ui *ui, Step *step);
void hidden_set_colorise(ColorPair colors[81][9], Step *step);

#endif
