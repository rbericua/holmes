#ifndef BITS_H
#define BITS_H

#define BIT(n) (1u << (n))
#define SET_BIT(x, n) ((x) | BIT(n))
#define UNSET_BIT(x, n) ((x) & ~BIT(n))
#define IS_BIT_SET(x, n) ((x) & BIT(n))

int count_ones(unsigned int x);
int find_first_set(unsigned int x);

#endif
