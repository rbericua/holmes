#ifndef EXPLAIN_H
#define EXPLAIN_H

#include "cand_set.h"
#include "geometry.h"

char *explain_unit_name(UnitType unit);
char *explain_unit_name_plural(UnitType unit);
char *explain_set_name(int size);
char *explain_fish_name(int size);
char *explain_units(int units[3]);
char *explain_cand_set(CandSet cands);
char *explain_cell(int cell);
char *explain_cells(int cells[], int num_cells);
char *explain_nums_plus_one(int nums[], int len);
char *explain_value_removal(int cell, int value);
char *explain_cands_removal(int cell, CandSet cands);

#endif
