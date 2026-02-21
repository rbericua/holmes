#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdbool.h>

#include "grid.h"
#include "step.h"
#include "ui.h"

typedef bool (*TechniqueFn)(Grid *, Step *);

typedef struct {
    void (*apply)(Grid *, Step *);
    void (*revert)(Grid *, Step *);
    void (*explain)(Ui *, Step *);
    void (*colorise)(ColorPair[81][9], Step *);
} TechniqueOps;

extern TechniqueFn techniques[];
extern TechniqueOps technique_ops[];

#endif
