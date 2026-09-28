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
    Bitset##n bitset##n##_not(Bitset##n set) { \
        return ~set; \
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

Bitset128 bitset128_from_value(int value) {
    Bitset128 ret = BITSET128_EMPTY;
    ret.bits[value / 64] |= BIT(value % 64);
    return ret;
}

Bitset128 bitset128_from_arr(int arr[], int len) {
    Bitset128 set = BITSET128_EMPTY;
    for (int i = 0; i < len; i++) {
        set = bitset128_add(set, arr[i]);
    }
    return set;
}

int bitset128_to_arr(Bitset128 set, int arr[]) {
    int count = 0;
    for (int pos = 0; pos < 128; pos++) {
        if (bitset128_has(set, pos)) {
            arr[count++] = pos;
        }
    }
    return count;
}

bool bitset128_is_empty(Bitset128 set) {
    return set.bits[0] == 0 && set.bits[1] == 0;
}

int bitset128_len(Bitset128 set) {
    return __builtin_popcountll(set.bits[0])
           + __builtin_popcountll(set.bits[1]);
}

bool bitset128_has(Bitset128 set, int pos) {
    return IS_BIT_SET(set.bits[pos / 64], pos % 64);
}

Bitset128 bitset128_add(Bitset128 set, int pos) {
    int bucket = pos / 64;
    set.bits[bucket] = SET_BIT(set.bits[bucket], pos % 64);
    return set;
}

Bitset128 bitset128_remove(Bitset128 set, int pos) {
    int bucket = pos / 64;
    set.bits[bucket] = UNSET_BIT(set.bits[bucket], pos % 64);
    return set;
}

bool bitset128_is_sub(Bitset128 set1, Bitset128 set2) {
    return ((set1.bits[0] & set2.bits[0]) == set1.bits[0])
           && ((set1.bits[1] & set2.bits[1]) == set1.bits[1]);
}

int bitset128_first(Bitset128 set) {
    int first = __builtin_ffsll(set.bits[0]);
    if (first != 0) return first - 1;
    return __builtin_ffsll(set.bits[1]) + 63;
}

int bitset128_pop(Bitset128 *set) {
    int first = bitset128_first(*set);
    *set = bitset128_remove(*set, first);
    return first;
}

Bitset128 bitset128_not(Bitset128 set) {
    return (Bitset128){.bits = {~set.bits[0], ~set.bits[1]}};
}

Bitset128 bitset128_intersection(Bitset128 sets[], int num_sets) {
    Bitset128 result = BITSET128_FULL;
    for (int i = 0; i < num_sets; i++) {
        result.bits[0] &= sets[i].bits[0];
        result.bits[1] &= sets[i].bits[1];
    }
    return result;
}

Bitset128 bitset128_union(Bitset128 sets[], int num_sets) {
    Bitset128 result = BITSET128_EMPTY;
    for (int i = 0; i < num_sets; i++) {
        result.bits[0] |= sets[i].bits[0];
        result.bits[1] |= sets[i].bits[1];
    }
    return result;
}
