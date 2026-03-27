#ifndef STEP_H
#define STEP_H

#include "geometry.h"

typedef enum {
    TECH_NAKED_SINGLE,

    NUM_TECHNIQUES
} TechniqueType;

#define MAX_NAKED_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int cell;
    int value;
    int removal_cells[MAX_NAKED_SINGLE_REMOVALS];
    int num_removals;
} NakedSingleStep;

typedef struct {
    TechniqueType type;
    union {
        NakedSingleStep naked_single;
    } as;
} Step;

#endif
