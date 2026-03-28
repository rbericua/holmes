#ifndef CAND_SET_H
#define CAND_SET_H

#include <stdbool.h>

#define CAND_SET_EMPTY 0x0
#define CAND_SET_FULL 0x1ff

int cand_set_to_arr(unsigned int set, int arr[]);
int cand_set_len(unsigned int set);
bool cand_set_has(unsigned int set, int cand);
void cand_set_add(unsigned int *set, int cand);
void cand_set_remove(unsigned int *set, int cand);
void cand_set_clear(unsigned int *set);
int cand_set_first(unsigned int set);
unsigned int cand_set_intersection(unsigned int sets[], int num_sets);
unsigned int cand_set_union(unsigned int sets[], int num_sets);

#endif
