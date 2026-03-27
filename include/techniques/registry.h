#ifndef REGISTRY_H
#define REGISTRY_H

#include "grid.h"
#include "step.h"
#include "ui.h"

typedef bool (*TechniqueFn)(Grid *, Step *);

typedef struct {
    void (*apply)(Grid *, Step *);
    void (*colorise)(ColorPair[81][9], Step *);
} TechniqueOps;

extern TechniqueFn techniques[NUM_TECHNIQUES];
extern TechniqueOps technique_ops[NUM_TECHNIQUES];

#endif
