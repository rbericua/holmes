#ifndef STEP_H
#define STEP_H

#include "geometry.h"

typedef enum {
    TECH_NAKED_SINGLE,
    TECH_HIDDEN_SINGLE,
    TECH_NAKED_PAIR,
    TECH_NAKED_TRIPLE,
    TECH_NAKED_QUAD,

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

#define MAX_NAKED_SET_SIZE 4
#define MAX_NAKED_SET_REMOVALS MAX_COMMON_PEERS

typedef struct {
    int set_cells[MAX_NAKED_SET_SIZE];
    int set_size;
    unsigned int set_cands;
    int removal_cells[MAX_NAKED_SET_REMOVALS];
    unsigned int removal_cands[MAX_NAKED_SET_REMOVALS];
    int num_removals;
    int units[3];
} NakedSetStep;

typedef struct {
    TechniqueType type;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
        NakedSetStep naked_set;
    } as;
} Step;

#endif
