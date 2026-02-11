#ifndef STEP_H
#define STEP_H

#include "cand_set.h"
#include "grid.h"

typedef enum {
    TECH_NAKED_SINGLE,
    TECH_HIDDEN_SINGLE,
    TECH_NAKED_SET,

    NUM_TECHNIQUES
} TechniqueType;

typedef struct {
    int idx;
    int value;
} NakedSingleStep;

typedef struct {
    int idx;
    int value;
    UnitType unit_type;
    int unit_idx;
} HiddenSingleStep;

typedef struct {
    int set_idxs[4];
    int set_size;
    CandSet set_cands;
    int removal_idxs[MAX_COMMON_PEERS];
    CandSet removed_cands[MAX_COMMON_PEERS];
    int num_removals;
    UnitType unit_type;
    int unit_idx;
} NakedSetStep;

typedef struct {
    TechniqueType tech;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
        NakedSetStep naked_set;
    } as;
} Step;

#endif
