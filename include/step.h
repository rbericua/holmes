#ifndef STEP_H
#define STEP_H

#include "cand_set.h"
#include "grid.h"

typedef enum {
    TECH_NAKED_SINGLE,
    TECH_HIDDEN_SINGLE,
    TECH_NAKED_PAIR,
    TECH_NAKED_TRIPLE,
    TECH_NAKED_QUAD,
    TECH_HIDDEN_PAIR,
    TECH_HIDDEN_TRIPLE,
    TECH_HIDDEN_QUAD,
    TECH_POINTING_SET,
    TECH_X_WING,
    TECH_SWORDFISH,
    TECH_JELLYFISH,

    NUM_TECHNIQUES
} TechniqueType;

#define MAX_NAKED_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int idx;
    int value;
    int removal_idxs[MAX_NAKED_SINGLE_REMOVALS];
    int num_removals;
} NakedSingleStep;

#define MAX_HIDDEN_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int idx;
    int value;
    int removal_idxs[MAX_HIDDEN_SINGLE_REMOVALS];
    int num_removals;
    CandSet old_cands;
    UnitType unit_type;
    int unit_idx;
} HiddenSingleStep;

#define MAX_NAKED_SET_SIZE 4
#define MAX_NAKED_SET_REMOVALS MAX_COMMON_PEERS

typedef struct {
    int idxs[MAX_NAKED_SET_SIZE];
    int size;
    CandSet cands;
    int removal_idxs[MAX_NAKED_SET_REMOVALS];
    CandSet removed_cands[MAX_NAKED_SET_REMOVALS];
    int num_removals;
    UnitType unit_type;
    int unit_idx;
} NakedSetStep;

#define MAX_HIDDEN_SET_SIZE 4
#define MAX_HIDDEN_SET_REMOVALS 4

typedef struct {
    int idxs[MAX_HIDDEN_SET_SIZE];
    int size;
    CandSet cands;
    int removal_idxs[MAX_HIDDEN_SET_REMOVALS];
    CandSet removed_cands[MAX_HIDDEN_SET_REMOVALS];
    int num_removals;
    UnitType unit_type;
    int unit_idx;
} HiddenSetStep;

#define MAX_POINTING_SET_SIZE 3
#define MAX_POINTING_SET_REMOVALS 6

typedef struct {
    int idxs[MAX_POINTING_SET_SIZE];
    int size;
    int value;
    int removal_idxs[MAX_POINTING_SET_REMOVALS];
    int num_removals;
    UnitType trigger_unit_type;
    int trigger_unit_idx;
    UnitType removal_unit_type;
    int removal_unit_idx;
} PointingSetStep;

#define MAX_BASIC_FISH_SIZE 4
#define MAX_BASIC_FISH_REMOVALS 20

typedef struct {
    int base_idxs[MAX_BASIC_FISH_SIZE];
    int cover_idxs[MAX_BASIC_FISH_SIZE];
    int size;
    int value;
    int removal_idxs[MAX_BASIC_FISH_REMOVALS];
    int num_removals;
    UnitType unit_type;
} BasicFishStep;

typedef struct {
    TechniqueType tech;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
        NakedSetStep naked_set;
        HiddenSetStep hidden_set;
        PointingSetStep pointing_set;
        BasicFishStep basic_fish;
    } as;
} Step;

typedef struct {
    Step *elems;
    int len;
    int cap;
} Steps;

#endif
