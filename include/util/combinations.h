#ifndef COMBINATIONS_H
#define COMBINATIONS_H

void *generate_combinations(void *arr, int arr_len, int comb_len, int elem_size,
                            int *out_num_combs);
void free_combinations(void *combs);

#endif
