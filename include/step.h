#ifndef STEP_H
#define STEP_H

#include "cand_set.h"
#include "geometry.h"

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
    TECH_FINNED_X_WING,
    TECH_FINNED_SWORDFISH,
    TECH_FINNED_JELLYFISH,

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
    CandSet old_cands;
    int removal_cells[MAX_HIDDEN_SINGLE_REMOVALS];
    int num_removals;
    int units[3];
} HiddenSingleStep;

#define MAX_NAKED_SET_SIZE 4
#define MAX_NAKED_SET_REMOVALS MAX_COMMON_PEERS

typedef struct {
    int set_cells[MAX_NAKED_SET_SIZE];
    int set_size;
    CandSet set_cands;
    int removal_cells[MAX_NAKED_SET_REMOVALS];
    CandSet removal_cands[MAX_NAKED_SET_REMOVALS];
    int num_removals;
    int units[3];
} NakedSetStep;

#define MAX_HIDDEN_SET_SIZE 4
#define MAX_HIDDEN_SET_REMOVALS MAX_HIDDEN_SET_SIZE

typedef struct {
    int set_cells[MAX_HIDDEN_SET_SIZE];
    int set_size;
    CandSet set_cands;
    CandSet removal_cands[MAX_HIDDEN_SET_REMOVALS];
    int num_removals;
    int units[3];
} HiddenSetStep;

#define MAX_POINTING_SET_SIZE 3
#define MAX_POINTING_SET_REMOVALS 6

typedef struct {
    int set_cells[MAX_POINTING_SET_SIZE];
    int set_size;
    int value;
    int removal_cells[MAX_POINTING_SET_REMOVALS];
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
    int fish_size;
    int value;
    int removal_cells[MAX_BASIC_FISH_REMOVALS];
    int num_removals;
    UnitType base_unit_type;
    UnitType cover_unit_type;
} BasicFishStep;

#define MAX_FINNED_FISH_SIZE 4
#define MAX_FINNED_FISH_REMOVALS 4
#define MAX_FINS 4

typedef struct {
    int base_idxs[MAX_FINNED_FISH_SIZE];
    int cover_idxs[MAX_FINNED_FISH_SIZE];
    int fins[MAX_FINS];
    int num_fins;
    int fish_size;
    int value;
    int removal_cells[MAX_FINNED_FISH_REMOVALS];
    int num_removals;
    UnitType base_unit_type;
    UnitType cover_unit_type;
} FinnedFishStep;

typedef struct {
    TechniqueType type;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
        NakedSetStep naked_set;
        HiddenSetStep hidden_set;
        PointingSetStep pointing_set;
        BasicFishStep basic_fish;
        FinnedFishStep finned_fish;
    } as;
} Step;

typedef struct {
    Step *elems;
    int len;
    int cap;
} Steps;

#endif
