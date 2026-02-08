#ifndef STEP_H
#define STEP_H

typedef enum {
    TECH_NAKED_SINGLE,

    NUM_TECHNIQUES
} TechniqueType;

typedef struct {
    TechniqueType tech;
    union {
        struct {
            int idx;
            int value;
        } naked_single;
    } as;
} Step;

#endif
