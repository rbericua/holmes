#ifndef CAND_SET_H
#define CAND_SET_H

#include <stdbool.h>

typedef struct {
    unsigned int cands;
    int len;
} CandSet;

CandSet cand_set_empty(void);
CandSet cand_set_full(void);
bool cand_set_has(CandSet set, int cand);
void cand_set_add(CandSet *set, int cand);
void cand_set_remove(CandSet *set, int cand);
void cand_set_clear(CandSet *set);
int cand_set_only(CandSet set);
CandSet cand_set_intersection(int num_sets, ...);
CandSet cand_set_union(int num_sets, ...);

#endif
