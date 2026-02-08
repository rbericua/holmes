#include "bits.h"

int count_ones(unsigned int x) {
    int count = 0;
    while (x) {
        count++;
        x &= x - 1;
    }
    return count;
}

int find_first_set(unsigned int x) {
    int value = 0;
    while (x) {
        value++;
        x >>= 1;
    }
    return value;
}
