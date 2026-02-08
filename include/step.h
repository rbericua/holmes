#ifndef STEP_H
#define STEP_H

#include "grid.h"

typedef enum {
    TECH_NAKED_SINGLE,
    TECH_HIDDEN_SINGLE,

    NUM_TECHNIQUES
} TechniqueType;

typedef struct {
    TechniqueType tech;
    union {
        struct {
            int idx;
            int value;
        } naked_single;
        struct {
            int idx;
            int value;
            UnitType unit_type;
            int unit_idx;
        } hidden_single;
    } as;
} Step;

#endif
