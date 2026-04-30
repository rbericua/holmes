#include "cand_set.h"

#include <assert.h>
#include <stdbool.h>

#include "util/bits.h"

unsigned int cand_set_from_value(int value) {
    return BIT(value - 1);
}

unsigned int cand_set_from_arr(int arr[], int len) {
    unsigned int set;
    for (int i = 0; i < len; i++) {
        cand_set_add(&set, arr[i]);
    }
    return set;
}

int cand_set_to_arr(unsigned int set, int arr[]) {
    int count = 0;
    for (int value = 1; value <= 9; value++) {
        if (cand_set_has(set, value)) {
            arr[count++] = value;
        }
    }
    return count;
}

int cand_set_len(unsigned int set) {
    return __builtin_popcount(set);
}

bool cand_set_has(unsigned int set, int cand) {
    return cand != 0 && IS_BIT_SET(set, cand - 1);
}

void cand_set_add(unsigned int *set, int cand) {
    if (cand == 0) return;
    *set = SET_BIT(*set, cand - 1);
}

void cand_set_remove(unsigned int *set, int cand) {
    if (cand == 0) return;
    *set = UNSET_BIT(*set, cand - 1);
}

void cand_set_clear(unsigned int *set) {
    *set = CAND_SET_EMPTY;
}

int cand_set_first(unsigned int set) {
    return __builtin_ffs(set);
}

unsigned int cand_set_difference(unsigned int a, unsigned int b) {
    return a & ~b;
}

unsigned int cand_set_intersection(unsigned int sets[], int num_sets) {
    unsigned int result = CAND_SET_FULL;
    for (int i = 0; i < num_sets; i++) {
        result &= sets[i];
    }
    return result;
}

unsigned int cand_set_union(unsigned int sets[], int num_sets) {
    unsigned int result = CAND_SET_EMPTY;
    for (int i = 0; i < num_sets; i++) {
        result |= sets[i];
    }
    return result;
}
