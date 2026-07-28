#ifndef STEP_H
#define STEP_H

#include <stdbool.h>

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
    TECH_Y_WING,
    TECH_SIMPLE_COLORING,
    TECH_MEDUSA,

    NUM_TECHNIQUES
} TechniqueType;

#define MAX_NAKED_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int cell;
    int value;
    int removal_cells[MAX_NAKED_SINGLE_REMOVALS];
    int num_removals;
} NakedSingleInfo;

typedef struct {
    NakedSingleInfo steps[81];
    int len;
} NakedSingleStep;

#define MAX_HIDDEN_SINGLE_REMOVALS NUM_PEERS

typedef struct {
    int cell;
    int value;
    CandSet old_cands;
    int removal_cells[MAX_HIDDEN_SINGLE_REMOVALS];
    int num_removals;
    int units[3];
} HiddenSingleInfo;

typedef struct {
    HiddenSingleInfo steps[81];
    int len;
} HiddenSingleStep;

#define MAX_NAKED_SET_SIZE 4
#define MAX_NAKED_SET_REMOVALS MAX_COMMON_PEERS

typedef struct {
    int set_cells[MAX_NAKED_SET_SIZE];
    int size;
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
    int size;
    CandSet set_cands;
    CandSet removal_cands[MAX_HIDDEN_SET_REMOVALS];
    int num_removals;
    int units[3];
} HiddenSetStep;

#define MAX_POINTING_SET_SIZE 3
#define MAX_POINTING_SET_REMOVALS 6

typedef struct {
    int set_cells[MAX_POINTING_SET_SIZE];
    int size;
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
    int size;
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
    int size;
    int value;
    int removal_cells[MAX_FINNED_FISH_REMOVALS];
    int num_removals;
    UnitType base_unit_type;
    UnitType cover_unit_type;
} FinnedFishStep;

#define MAX_Y_WING_REMOVALS 5

typedef struct {
    int hinge;
    CandSet hinge_cands;
    int wings[2];
    CandSet wing_cands[2];
    int removal_cand;
    int removal_cells[MAX_Y_WING_REMOVALS];
    int num_removals;
} YWingStep;

#define MAX_SIMPLE_COLORING_CHAIN_LEN 18
#define MAX_SIMPLE_COLORING_LINKS 27
#define MAX_SIMPLE_COLORING_REMOVALS 81

typedef enum {
    SC_TWICE_IN_UNIT,
    SC_BOTH_SEEN,
    SC_EMPTIED_UNIT
} SimpleColoringRule;

typedef struct {
    int cell;
    int color;
} SimpleColoringNode;

typedef struct {
    SimpleColoringNode chain[MAX_SIMPLE_COLORING_CHAIN_LEN];
    int chain_len;
    int links[MAX_SIMPLE_COLORING_LINKS][2];
    int num_links;
    int value;
    SimpleColoringRule rule;
    int removal_color;
    union {
        struct {
            int cells[2];
        } twice_in_unit;
        struct {
            int removal_cells[MAX_SIMPLE_COLORING_REMOVALS];
            int num_removals;
        } both_seen;
        struct {
            int unit_idx;
            UnitType unit_type;
            int emptied_cells[9];
            int num_emptied_cells;
        } emptied_unit;
    };
} SimpleColoringStep;

#define MAX_MEDUSA_CHAIN_LEN 729
#define MAX_MEDUSA_LINKS 324
#define MAX_MEDUSA_REMOVALS 729

typedef struct {
    int cell1;
    int cand1;
    int cell2;
    int cand2;
} Link;

typedef enum {
    MED_TWICE_IN_CELL,
    MED_TWICE_IN_UNIT,
    MED_BOTH_SEEN,
    MED_EMPTIED_CELL,
    MED_EMPTIED_UNIT
} MedusaRule;

typedef enum {
    MED_BOTH_SEEN_CELL,
    MED_BOTH_SEEN_UNITS,
    MED_BOTH_SEEN_CELL_UNIT,
} MedusaBothSeenRule;

typedef struct {
    int cell;
    int cand;
    int color;
} MedusaNode;

typedef struct {
    int cell;
    int cand;
    MedusaBothSeenRule rule;
} MedusaRemoval;

typedef struct {
    MedusaNode chain[MAX_MEDUSA_CHAIN_LEN];
    int chain_len;
    Link links[MAX_MEDUSA_LINKS];
    int num_links;
    MedusaRule rule;
    int removal_color;
    union {
        struct {
            int cell;
        } twice_in_cell;
        struct {
            int value;
            int cells[2];
        } twice_in_unit;
        struct {
            MedusaRemoval removals[MAX_MEDUSA_REMOVALS];
            int num_removals;
        } both_seen;
        struct {
            int cell;
        } emptied_cell;
        struct {
            int value;
            int unit_idx;
            UnitType unit_type;
            int emptied_cells[9];
            int num_emptied_cells;
        } emptied_unit;
    };
} MedusaStep;

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
        YWingStep y_wing;
        SimpleColoringStep simple_coloring;
        MedusaStep medusa;
    } as;
} Step;

typedef struct {
    Step *elems;
    int len;
    int cap;
} Steps;

#endif
