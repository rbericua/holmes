#include "util/bits.h"

#include <stdbool.h>

#define BITSET_N_IMPLEMENTATIONS(n) \
    Bitset##n bitset##n##_from_value(int value) { \
        return BIT(value); \
    } \
    Bitset##n bitset##n##_from_arr(int arr[], int len) { \
        Bitset##n set = BITSET##n##_EMPTY; \
        for (int i = 0; i < len; i++) { \
            set = bitset##n##_add(set, arr[i]); \
        } \
        return set; \
    } \
    int bitset##n##_to_arr(Bitset##n set, int arr[]) { \
        int count = 0; \
        for (int pos = 0; pos < n; pos++) { \
            if (bitset##n##_has(set, pos)) { \
                arr[count++] = pos; \
            } \
        } \
        return count; \
    } \
    bool bitset##n##_is_empty(Bitset##n set) { \
        return set == BITSET##n##_EMPTY; \
    } \
    int bitset##n##_len(Bitset##n set) { \
        return __builtin_popcountll(set); \
    } \
    bool bitset##n##_has(Bitset##n set, int pos) { \
        return IS_BIT_SET(set, pos); \
    } \
    Bitset##n bitset##n##_add(Bitset##n set, int pos) { \
        return SET_BIT(set, pos); \
    } \
    Bitset##n bitset##n##_remove(Bitset##n set, int pos) { \
        return UNSET_BIT(set, pos); \
    } \
    bool bitset##n##_is_sub(Bitset##n set1, Bitset##n set2) { \
        return (set1 & set2) == set1; \
    } \
    int bitset##n##_first(Bitset##n set) { \
        return __builtin_ffsll(set) - 1; \
    } \
    int bitset##n##_pop(Bitset##n *set) { \
        int first = bitset##n##_first(*set); \
        *set = bitset##n##_remove(*set, first); \
        return first; \
    } \
    Bitset##n bitset##n##_intersection(Bitset##n sets[], int num_sets) { \
        Bitset##n result = BITSET##n##_FULL; \
        for (int i = 0; i < num_sets; i++) { \
            result &= sets[i]; \
        } \
        return result; \
    } \
    Bitset##n bitset##n##_union(Bitset##n sets[], int num_sets) { \
        Bitset##n result = BITSET##n##_EMPTY; \
        for (int i = 0; i < num_sets; i++) { \
            result |= sets[i]; \
        } \
        return result; \
    }

BITSET_N_IMPLEMENTATIONS(8)
BITSET_N_IMPLEMENTATIONS(16)
BITSET_N_IMPLEMENTATIONS(32)
BITSET_N_IMPLEMENTATIONS(64)
