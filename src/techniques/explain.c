#include "techniques/explain.h"

#include "cand_set.h"
#include "cell.h"
#include "dynstr.h"

void print_cand_set(DynStr *ds, CandSet set) {
    int arr[9];
    cand_set_to_arr(set, arr);

    ds_appendf(ds, "{");
    for (int i = 0; i < set.len; i++) {
        ds_appendf(ds, "%d", arr[i]);
        if (i < set.len - 1) {
            ds_appendf(ds, ", ");
        }
    }
    ds_appendf(ds, "}");
}

void print_idxs(DynStr *ds, int idxs[], int num_idxs) {
    for (int i = 0; i < num_idxs; i++) {
        int row = ROW_FROM_IDX(idxs[i]);
        int col = COL_FROM_IDX(idxs[i]);

        ds_appendf(ds, "r%dc%d", row + 1, col + 1);
        if (i < num_idxs - 1) {
            ds_appendf(ds, ", ");
        }
    }
}
