#include "techniques/registry.h"

#include "step.h"

#include "techniques/naked_single.h"

#define TECHNIQUE_OPS(tech) \
    { \
        .apply = tech##_apply, \
        .explain = tech##_explain, \
        .colorise = tech##_colorise, \
    }

TechniqueFn techniques[] = {naked_single};

TechniqueOps technique_ops[] = {
    [TECH_NAKED_SINGLE] = TECHNIQUE_OPS(naked_single),
};
