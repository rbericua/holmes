#include "util/bits.h"

#include <stdbool.h>

#define BITSET_N_IMPLEMENTATIONS(n) \
    Bitset##n bitset##n##_from_value(int value) { \
        return BIT(value); \
    } \
    Bitset##n bitset##n##_from_arr(int arr[], int len) { \
        Bitset##n set; \
        for (int i = 0; i < len; i++) { \
            bitset##n##_add(&set, arr[i]); \
        } \
        return set; \
    } \
    int bitset##n##_to_arr(Bitset##n set, int arr[]) { \
        int count = 0; \
        for (int value = 0; value < 8; value++) { \
            if (bitset##n##_has(set, value)) { \
                arr[count++] = value; \
            } \
        } \
        return count; \
    } \
    int bitset##n##_len(Bitset##n set) { \
        return __builtin_popcount(set); \
    } \
    bool bitset##n##_has(Bitset##n set, int value) { \
        return IS_BIT_SET(set, value); \
    } \
    void bitset##n##_add(Bitset##n *set, int value) { \
        *set = SET_BIT(*set, value); \
    } \
    void bitset##n##_remove(Bitset##n *set, int value) { \
        *set = UNSET_BIT(*set, value); \
    } \
    void bitset##n##_clear(Bitset##n *set) { \
        *set = BITSET##n##_EMPTY; \
    } \
    int bitset##n##_first(Bitset##n set) { \
        return __builtin_ffs(set); \
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
