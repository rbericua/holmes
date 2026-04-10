#ifndef STEP_H
#define STEP_H

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

#define MAX_HIDDEN_SET_SIZE 4
#define MAX_HIDDEN_SET_REMOVALS MAX_HIDDEN_SET_SIZE

typedef struct {
    int set_cells[MAX_HIDDEN_SET_SIZE];
    int set_size;
    unsigned int set_cands;
    int removal_cells[MAX_HIDDEN_SET_REMOVALS];
    unsigned int removal_cands[MAX_HIDDEN_SET_REMOVALS];
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

typedef struct {
    TechniqueType type;
    union {
        NakedSingleStep naked_single;
        HiddenSingleStep hidden_single;
        NakedSetStep naked_set;
        HiddenSetStep hidden_set;
        PointingSetStep pointing_set;
    } as;
} Step;

#endif
