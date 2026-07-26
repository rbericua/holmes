#ifndef CAND_SET_H
#define CAND_SET_H

#include <stdbool.h>
#include <stdint.h>

#define CAND_SET_EMPTY 0x0
#define CAND_SET_FULL 0x1ff

typedef uint16_t CandSet;

CandSet cand_set_from_value(int value);
CandSet cand_set_from_arr(int arr[], int len);
int cand_set_to_arr(CandSet set, int arr[]);
int cand_set_len(CandSet set);
bool cand_set_has(CandSet set, int cand);
CandSet cand_set_add(CandSet set, int cand);
CandSet cand_set_remove(CandSet set, int cand);
int cand_set_first(CandSet set);
CandSet cand_set_intersection(CandSet sets[], int num_sets);
CandSet cand_set_union(CandSet sets[], int num_sets);

#endif
