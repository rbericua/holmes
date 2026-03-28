#include "techniques/explain.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "util/dynstr.h"

char *explain_unit(UnitType unit) {
    switch (unit) {
    case UNIT_ROW: return strdup("Row");
    case UNIT_COL: return strdup("Column");
    case UNIT_BOX: return strdup("Box");
    default: return "";
    }
}

char *explain_units(int units[3]) {
    DynStr ds = {0};

    bool first = true;
    for (int i = 0; i < 3; i++) {
        if (units[i] != -1) {
            char *unit_str = explain_unit(i);
            ds_append(&ds, "%s%s %d", first ? "" : "/", unit_str, units[i] + 1);
            free(unit_str);

            first = false;
        }
    }

    return ds.elems;
}
