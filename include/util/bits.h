#ifndef BITS_H
#define BITS_H

#include <stdbool.h>
#include <stdint.h>

#define BIT(n) (1ull << (n))
#define SET_BIT(x, n) ((x) | BIT(n))
#define UNSET_BIT(x, n) ((x) & ~BIT(n))
#define IS_BIT_SET(x, n) ((x) & BIT(n))

#define BITSET_N_DECLARATIONS(n) \
    typedef uint##n##_t Bitset##n; \
    Bitset##n bitset##n##_from_value(int value); \
    Bitset##n bitset##n##_from_arr(int arr[], int len); \
    int bitset##n##_to_arr(Bitset##n set, int arr[]); \
    int bitset##n##_len(Bitset##n set); \
    bool bitset##n##_has(Bitset##n set, int pos); \
    Bitset##n bitset##n##_add(Bitset##n set, int pos); \
    Bitset##n bitset##n##_remove(Bitset##n set, int pos); \
    bool bitset##n##_is_sub(Bitset##n set1, Bitset##n set2); \
    int bitset##n##_first(Bitset##n set); \
    Bitset##n bitset##n##_intersection(Bitset##n sets[], int num_sets); \
    Bitset##n bitset##n##_union(Bitset##n sets[], int num_sets);

BITSET_N_DECLARATIONS(8);
#define BITSET8_EMPTY 0
#define BITSET8_FULL UINT8_MAX

BITSET_N_DECLARATIONS(16);
#define BITSET16_EMPTY 0
#define BITSET16_FULL UINT16_MAX

BITSET_N_DECLARATIONS(32);
#define BITSET32_EMPTY 0
#define BITSET32_FULL UINT32_MAX

BITSET_N_DECLARATIONS(64);
#define BITSET64_EMPTY 0
#define BITSET64_FULL UINT64_MAX

#endif
