#include "bits.h"

int count_ones(unsigned int x) {
    int count = 0;
    while (x) {
        count++;
        x &= x - 1;
    }
    return count;
}
