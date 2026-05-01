#include "cand_set.h"

#include <assert.h>
#include <stdbool.h>

#include "util/bits.h"

CandSet cand_set_from_value(int value) {
    return BIT(value - 1);
}

CandSet cand_set_from_arr(int arr[], int len) {
    CandSet set;
    for (int i = 0; i < len; i++) {
        cand_set_add(&set, arr[i]);
    }
    return set;
}

int cand_set_to_arr(CandSet set, int arr[]) {
    int count = 0;
    for (int value = 1; value <= 9; value++) {
        if (cand_set_has(set, value)) {
            arr[count++] = value;
        }
    }
    return count;
}

int cand_set_len(CandSet set) {
    return __builtin_popcount(set);
}

bool cand_set_has(CandSet set, int cand) {
    return cand != 0 && IS_BIT_SET(set, cand - 1);
}

void cand_set_add(CandSet *set, int cand) {
    if (cand == 0) return;
    *set = SET_BIT(*set, cand - 1);
}

void cand_set_remove(CandSet *set, int cand) {
    if (cand == 0) return;
    *set = UNSET_BIT(*set, cand - 1);
}

void cand_set_clear(CandSet *set) {
    *set = CAND_SET_EMPTY;
}

int cand_set_first(CandSet set) {
    return __builtin_ffs(set);
}

CandSet cand_set_intersection(CandSet sets[], int num_sets) {
    CandSet result = CAND_SET_FULL;
    for (int i = 0; i < num_sets; i++) {
        result &= sets[i];
    }
    return result;
}

CandSet cand_set_union(CandSet sets[], int num_sets) {
    CandSet result = CAND_SET_EMPTY;
    for (int i = 0; i < num_sets; i++) {
        result |= sets[i];
    }
    return result;
}
