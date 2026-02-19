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

CandSet cand_set_from_mask(unsigned int mask) {
    CandSet set;
    set.cands = mask;
    set.len = count_ones(set.cands);
    return set;
}

CandSet cand_set_from_values(int num_values, ...) {
    CandSet set;

    va_list values;
    va_start(values, num_values);

    for (int i = 0; i < num_values; i++) {
        cand_set_add(&set, va_arg(values, int));
    }

    va_end(values);

    return set;
}

CandSet cand_set_from_arr(int arr[], int arr_len) {
    CandSet set = cand_set_empty();
    for (int i = 0; i < arr_len; i++) {
        cand_set_add(&set, arr[i]);
    }
    return set;
}

int cand_set_to_arr(CandSet set, int out[]) {
    int count = 0;
    for (int value = 1; value <= 9; value++) {
        if (cand_set_has(set, value)) {
            out[count++] = value;
        }
    }
    return count;
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

int cand_set_only(CandSet set) {
    return find_first_set(set.cands);
}

CandSet cand_set_difference(CandSet a, CandSet b) {
    CandSet result;
    result.cands = a.cands & ~b.cands;
    result.len = count_ones(result.cands);
    return result;
}

CandSet cand_set_intersection_from_va(int num_sets, ...) {
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

CandSet cand_set_intersection_from_arr(CandSet sets[], int num_sets) {
    CandSet result = cand_set_full();

    for (int i = 0; i < num_sets; i++) {
        result.cands &= sets[i].cands;
    }

    result.len = count_ones(result.cands);

    return result;
}

CandSet cand_set_union_from_va(int num_sets, ...) {
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

CandSet cand_set_union_from_arr(CandSet sets[], int num_sets) {
    CandSet result = cand_set_empty();

    for (int i = 0; i < num_sets; i++) {
        result.cands |= sets[i].cands;
    }

    result.len = count_ones(result.cands);

    return result;
}
