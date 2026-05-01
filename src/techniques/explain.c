#include "techniques/explain.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "geometry.h"
#include "util/dynstr.h"

char *explain_unit_name(UnitType unit) {
    switch (unit) {
    case UNIT_ROW: return strdup("Row");
    case UNIT_COL: return strdup("Column");
    case UNIT_BOX: return strdup("Box");
    default: return "";
    }
}

char *explain_set_name(int set_size) {
    switch (set_size) {
    case 2: return strdup("Pair");
    case 3: return strdup("Triple");
    case 4: return strdup("Quad");
    default: return "";
    }
}

char *explain_units(int units[3]) {
    DynStr ds = {0};

    bool first = true;
    for (int i = 0; i < 3; i++) {
        if (units[i] != -1) {
            char *unit_str = explain_unit_name(i);
            ds_append(&ds, "%s%s %d", first ? "" : "/", unit_str, units[i] + 1);
            free(unit_str);

            first = false;
        }
    }

    return ds.elems;
}

char *explain_cand_set(CandSet cands) {
    DynStr ds = {0};

    int cands_arr[9];
    int num_cands = cand_set_to_arr(cands, cands_arr);

    ds_append(&ds, "{");
    for (int i = 0; i < num_cands; i++) {
        ds_append(&ds, "%d%s", cands_arr[i], i == num_cands - 1 ? "" : ", ");
    }
    ds_append(&ds, "}");

    return ds.elems;
}

char *explain_cells(int cells[], int num_cells) {
    DynStr ds = {0};

    for (int i = 0; i < num_cells; i++) {
        int row = cell_row(cells[i]) + 1;
        int col = cell_col(cells[i]) + 1;
        ds_append(&ds, "r%dc%d%s", row, col, i == num_cells - 1 ? "" : ", ");
    }

    return ds.elems;
}

char *explain_value_removal(int cell, int value) {
    DynStr ds = {0};

    int row = cell_row(cell) + 1;
    int col = cell_col(cell) + 1;

    ds_append(&ds, "- Removed {%d} from r%dc%d", value, row, col);

    return ds.elems;
}

char *explain_cands_removal(int cell, CandSet cands) {
    DynStr ds = {0};

    char *cands_str = explain_cand_set(cands);
    int row = cell_row(cell) + 1;
    int col = cell_col(cell) + 1;

    ds_append(&ds, "- Removed %s from r%dc%d", cands_str, row, col);

    free(cands_str);

    return ds.elems;
}
