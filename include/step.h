#ifndef STEP_H
#define STEP_H

#include "geometry.h"

typedef enum {
    TECH_NAKED_SINGLE,
    TECH_HIDDEN_SINGLE,

    NUM_TECHNIQUES
} TechniqueType;

#define MAX_NAKED_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int cell;
    int value;
    int removal_cells[MAX_NAKED_SINGLE_REMOVALS];
    int num_removals;
} NakedSingleStep;

#define MAX_HIDDEN_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int cell;
    int value;
    int removal_cells[MAX_HIDDEN_SINGLE_REMOVALS];
    int num_removals;
    int units[3];
} HiddenSingleStep;

typedef struct {
    TechniqueType type;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
    } as;
} Step;

#endif
