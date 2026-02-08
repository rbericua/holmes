#include "cand_set.h"

#include <stdarg.h>
#include <stdbool.h>

#include "bits.h"

#define NO_CANDS 0x0
#define ALL_CANDS 0x1ff

CandSet cand_set_empty(void) {
    return (CandSet){.cands = NO_CANDS, .len = 0};
}

CandSet cand_set_full(void) {
    return (CandSet){.cands = ALL_CANDS, .len = 9};
}

bool cand_set_has(CandSet set, int cand) {
    if (cand == 0) return false;
    return IS_BIT_SET(set.cands, cand - 1);
}

void cand_set_add(CandSet *set, int cand) {
    if (cand_set_has(*set, cand) || cand == 0) return;
    set->cands = SET_BIT(set->cands, cand - 1);
    set->len++;
}

void cand_set_remove(CandSet *set, int cand) {
    if (!cand_set_has(*set, cand)) return;
    set->cands = UNSET_BIT(set->cands, cand - 1);
    set->len--;
}

void cand_set_clear(CandSet *set) {
    set->cands = NO_CANDS;
    set->len = 0;
}

CandSet cand_set_intersection(int num_sets, ...) {
    CandSet result = cand_set_full();

    va_list sets;
    va_start(sets, num_sets);

    for (int i = 0; i < num_sets; i++) {
        result.cands &= va_arg(sets, CandSet).cands;
    }

    va_end(sets);

    result.len = count_ones(result.cands);

    return result;
}

CandSet cand_set_union(int num_sets, ...) {
    CandSet result = cand_set_empty();

    va_list sets;
    va_start(sets, num_sets);

    for (int i = 0; i < num_sets; i++) {
        result.cands |= va_arg(sets, CandSet).cands;
    }

    va_end(sets);

    result.len = count_ones(result.cands);

    return result;
}
