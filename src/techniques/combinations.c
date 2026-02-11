#include <stdlib.h>
#include <string.h>

static int ncr(int n, int r);

void *generate_combinations(void *_arr, int arr_len, int comb_len,
                            int elem_size, int *out_num_combs) {
    char *arr = _arr;

    int num_combs = ncr(arr_len, comb_len);
    *out_num_combs = num_combs;

    char **combs = malloc(num_combs * sizeof(char *));
    combs[0] = malloc(num_combs * comb_len * elem_size);
    for (int i = 1; i < num_combs; i++) {
        combs[i] = combs[0] + i * comb_len * elem_size;
    }

    int *idxs = malloc(comb_len * sizeof(int));
    for (int i = 0; i < comb_len; i++) {
        idxs[i] = i;
    }

    for (int i = 0; i < num_combs; i++) {
        for (int j = 0; j < comb_len; j++) {
            memcpy(combs[0] + (i * comb_len + j) * elem_size,
                   arr + idxs[j] * elem_size, elem_size);
        }

        int j = comb_len - 1;
        while (j > 0 && idxs[j] == arr_len - comb_len + j) {
            j--;
        }

        idxs[j]++;
        for (j++; j < comb_len; j++) {
            idxs[j] = idxs[j - 1] + 1;
        }
    }

    free(idxs);

    return combs;
}

void free_combinations(void *_combs) {
    char **combs = _combs;
    free(combs[0]);
    free(combs);
}

static int ncr(int n, int r) {
    if (n - r < r) {
        r = n - r;
    }

    int result = 1;

    for (int i = 1; i <= r; i++) {
        result *= n - i + 1;
        result /= i;
    }

    return result;
}
