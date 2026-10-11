#include "bruteforce/patterns.h"

#include <stdlib.h>

#include "geometry.h"
#include "util/bits.h"

Bitset128 row_bits[9];
Bitset128 col_bits[9];
Bitset128 box_bits[9];

static void init_units(void);
static void generate_patterns_impl(Bitset128 pattern, Bitset128 occupied,
                                   int row, Bitset128 out[], int *out_idx);

Bitset128 *generate_patterns(void) {
    int idx = 0;
    Bitset128 *patterns = malloc(NUM_PATTERNS * sizeof(Bitset128));
    init_units();
    generate_patterns_impl(BITSET128_EMPTY, BITSET128_EMPTY, 0, patterns, &idx);
    return patterns;
}

void free_patterns(Bitset128 *patterns) {
    free(patterns);
}

static void init_units(void) {
    for (int i = 0; i < 9; i++) {
        row_bits[i] = BITSET128_EMPTY;
        col_bits[i] = BITSET128_EMPTY;
        box_bits[i] = BITSET128_EMPTY;
    }

    for (int i = 0; i < 81; i++) {
        int row = cell_row(i);
        int col = cell_col(i);
        int box = cell_box(i);

        row_bits[row] = bitset128_add(row_bits[row], i);
        col_bits[col] = bitset128_add(col_bits[col], i);
        box_bits[box] = bitset128_add(box_bits[box], i);
    }
}

static void generate_patterns_impl(Bitset128 pattern, Bitset128 occupied,
                                   int row, Bitset128 out[], int *out_idx) {
    if (row == 9) {
        out[(*out_idx)++] = pattern;
        return;
    }

    int next_cell = bitset128_first(bitset128_not(occupied));
    while (cell_row(next_cell) == row) {
        Bitset128 next_pattern = bitset128_add(pattern, next_cell);
        occupied = bitset128_add(occupied, next_cell);

        Bitset128 next_occupied_sets[] = {
            occupied, row_bits[cell_row(next_cell)],
            col_bits[cell_col(next_cell)], box_bits[cell_box(next_cell)]};
        Bitset128 next_occupied = bitset128_union(next_occupied_sets, 4);

        generate_patterns_impl(next_pattern, next_occupied, row + 1, out,
                               out_idx);

        next_cell = bitset128_first(bitset128_not(occupied));
    }
}
