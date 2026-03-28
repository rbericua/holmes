#ifndef EXPLAIN_H
#define EXPLAIN_H

#include "geometry.h"

char *explain_unit(UnitType unit);
char *explain_set(int set_size);
char *explain_units(int units[3]);
char *explain_cand_set(unsigned int cands);
char *explain_cells(int cells[], int num_cells);

#endif
