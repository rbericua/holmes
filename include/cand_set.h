#ifndef CAND_SET_H
#define CAND_SET_H

#include <stdbool.h>

typedef struct {
    unsigned int cands;
    int len;
} CandSet;

CandSet cand_set_empty(void);
CandSet cand_set_full(void);
CandSet cand_set_from_mask(unsigned int mask);
CandSet cand_set_from_arr(int arr[], int arr_len);
int cand_set_to_arr(CandSet set, int out[]);
bool cand_set_has(CandSet set, int cand);
void cand_set_add(CandSet *set, int cand);
void cand_set_remove(CandSet *set, int cand);
void cand_set_clear(CandSet *set);
int cand_set_only(CandSet set);
CandSet cand_set_difference(CandSet a, CandSet b);
CandSet cand_set_intersection_from_va(int num_sets, ...);
CandSet cand_set_intersection_from_arr(CandSet sets[], int num_sets);
CandSet cand_set_union_from_va(int num_sets, ...);
CandSet cand_set_union_from_arr(CandSet sets[], int num_sets);

#endif
