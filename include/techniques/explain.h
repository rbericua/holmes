#ifndef EXPLAIN_H
#define EXPLAIN_H

#include "cand_set.h"
#include "dynstr.h"

#define UNIT_TO_STR(u) \
    ((u) == UNIT_ROW ? "Row" : (u) == UNIT_COL ? "Column" : "Box")
#define UNIT_TO_STR_PLURAL(u) \
    ((u) == UNIT_ROW ? "Rows" : (u) == UNIT_COL ? "Columns" : "Boxs")
#define SET_NAME_FROM_SIZE(n) ((n) == 2 ? "Pair" : (n) == 3 ? "Triple" : "Quad")
#define FISH_NAME_FROM_SIZE(n) \
    ((n) == 2 ? "X-Wing" : (n) == 3 ? "Swordfish" : "Jellyfish")

void print_cand_set(DynStr *ds, CandSet set);
void print_idxs(DynStr *ds, int idxs[], int num_idxs);

#endif
