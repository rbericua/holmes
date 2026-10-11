#ifndef PATTERNS_H
#define PATTERNS_H

#include "util/bits.h"

#define NUM_PATTERNS (9 * 6 * 3 * 6 * 4 * 2 * 3 * 2 * 1)

Bitset128 *generate_patterns(void);
void free_patterns(Bitset128 *patterns);

#endif
