#include "techniques/registry.h"

#include "step.h"

#include "techniques/basic_fish.h"
#include "techniques/hidden_set.h"
#include "techniques/hidden_single.h"
#include "techniques/naked_set.h"
#include "techniques/naked_single.h"
#include "techniques/pointing_set.h"

#define TECHNIQUE_OPS(tech) \
    { \
        .apply = tech##_apply, \
        .revert = tech##_revert, \
        .explain = tech##_explain, \
        .colorise = tech##_colorise, \
    }

TechniqueFn techniques[] = {naked_single, hidden_single, naked_pair,
                            hidden_pair,  naked_triple,  hidden_triple,
                            naked_quad,   hidden_quad,   pointing_set,
                            x_wing,       swordfish,     jellyfish};

TechniqueOps technique_ops[] = {
    [TECH_NAKED_SINGLE] = TECHNIQUE_OPS(naked_single),
    [TECH_HIDDEN_SINGLE] = TECHNIQUE_OPS(hidden_single),
    [TECH_NAKED_PAIR] = TECHNIQUE_OPS(naked_set),
    [TECH_NAKED_TRIPLE] = TECHNIQUE_OPS(naked_set),
    [TECH_NAKED_QUAD] = TECHNIQUE_OPS(naked_set),
    [TECH_HIDDEN_PAIR] = TECHNIQUE_OPS(hidden_set),
    [TECH_HIDDEN_TRIPLE] = TECHNIQUE_OPS(hidden_set),
    [TECH_HIDDEN_QUAD] = TECHNIQUE_OPS(hidden_set),
    [TECH_POINTING_SET] = TECHNIQUE_OPS(pointing_set),
    [TECH_X_WING] = TECHNIQUE_OPS(basic_fish),
    [TECH_SWORDFISH] = TECHNIQUE_OPS(basic_fish),
    [TECH_JELLYFISH] = TECHNIQUE_OPS(basic_fish),
};
