#include "cand_set.h"

#include <assert.h>
#include <stdbool.h>

#include "util/bits.h"

int cand_set_len(unsigned int set) {
    return __builtin_popcount(set);
}

bool cand_set_has(unsigned int set, int cand) {
    return IS_BIT_SET(set, cand - 1);
}

void cand_set_add(unsigned int *set, int cand) {
    *set = SET_BIT(*set, cand - 1);
}

void cand_set_remove(unsigned int *set, int cand) {
    *set = UNSET_BIT(*set, cand - 1);
}

void cand_set_clear(unsigned int *set) {
    *set = CAND_SET_EMPTY;
}

int cand_set_first(unsigned int set) {
    return __builtin_ffs(set);
}
