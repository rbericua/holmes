#ifndef DYNARR_H
#define DYNARR_H

#include <stdlib.h>

#define DA_INIT_CAP 16
#define DA_GROWTH_FACTOR 2

#define da_init(da) \
    do { \
        (da)->elems = NULL; \
        (da)->len = 0; \
        (da)->cap = 0; \
    } while (0)

#define da_append(da, new_elem) \
    do { \
        da_reserve(da, (da)->len + 1); \
        (da)->elems[(da)->len++] = (new_elem); \
    } while (0)

#define da_reserve(da, required_cap) \
    do { \
        if ((da)->cap < (required_cap)) { \
            if ((da)->cap == 0) { \
                (da)->cap = DA_INIT_CAP; \
            } \
            while ((da)->cap < (required_cap)) { \
                (da)->cap *= DA_GROWTH_FACTOR; \
            } \
            (da)->elems = realloc((da)->elems, \
                                  (da)->cap * sizeof(*(da)->elems)); \
        } \
    } while (0)

#define da_clear(da) (da)->len = 0

#define da_deinit(da) free((da)->elems)

#endif
