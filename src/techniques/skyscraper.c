#include "techniques/skyscraper.h"

#include <stdbool.h>
#include <stdlib.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "techniques/explain.h"
#include "ui/colors.h"
#include "util/bits.h"
#include "util/combinations.h"
#include "util/dynstr.h"

typedef struct {
    int unit;
    int cells[2];
} ConjugatePair;

static bool skyscraper_unit(Grid *grid, Step *step, UnitType unit_type);
static int find_conjugate_pairs(Grid *grid, UnitType unit_type, int value,
                                ConjugatePair out[]);
static bool find_roof(ConjugatePair pairs[], UnitType unit_type, int roof[],
                      int *floor_unit);

bool skyscraper(Grid *grid, Step *step) {
    step->type = TECH_SKYSCRAPER;

    if (skyscraper_unit(grid, step, UNIT_ROW)) return true;
    if (skyscraper_unit(grid, step, UNIT_COL)) return true;
    return false;
}

void skyscraper_apply(Grid *grid, Step *step) {
    SkyscraperStep *s = &step->as.skyscraper;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->value);
    }
}

void skyscraper_revert(Grid *grid, Step *step) {
    SkyscraperStep *s = &step->as.skyscraper;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void skyscraper_explain(DynStr *buf, Step *step) {
    SkyscraperStep *s = &step->as.skyscraper;

    char *floor_str = explain_cells(s->floor, 2);
    char *roof_str = explain_cells(s->roof, 2);

    ds_append(buf, "[Skyscraper] {%d} on %s through %s\n", s->value, roof_str,
              floor_str);

    free(floor_str);
    free(roof_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->value);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void skyscraper_colorise(ColorPair colors[81][9], Step *step) {
    SkyscraperStep *s = &step->as.skyscraper;

    for (int i = 0; i < 2; i++) {
        colors[s->floor[i]][s->value - 1] = CP_SPECIAL1;
        colors[s->roof[i]][s->value - 1] = CP_TRIGGER;
    }

    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}

static bool skyscraper_unit(Grid *grid, Step *step, UnitType unit_type) {
    SkyscraperStep *s = &step->as.skyscraper;

    for (int value = 1; value <= 9; value++) {
        ConjugatePair pairs[9];
        int num_pairs = find_conjugate_pairs(grid, unit_type, value, pairs);
        if (num_pairs < 2) continue;

        int num_combs;
        ConjugatePair **combs = generate_combinations(
            pairs, num_pairs, 2, sizeof(ConjugatePair), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            ConjugatePair *comb = combs[comb_i];

            int roof[2];
            int floor_unit;
            if (!find_roof(comb, unit_type, roof, &floor_unit)) continue;

            int common_peers[MAX_COMMON_PEERS];
            int num_common_peers = cells_common_peers(roof, 2, common_peers);

            s->num_removals = grid_region_with_cand(
                grid, common_peers, num_common_peers, value, s->removal_cells);
            if (s->num_removals == 0) continue;

            for (int i = 0; i < 2; i++) {
                s->floor[i] = cell_from_unit_pos(comb[i].unit, floor_unit,
                                                 unit_type);
                s->roof[i] = roof[i];
            }
            s->value = value;

            free_combinations(combs);

            return true;
        }

        free_combinations(combs);
    }

    return false;
}

static int find_conjugate_pairs(Grid *grid, UnitType unit_type, int value,
                                ConjugatePair out[]) {
    int num_pairs = 0;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        int *unit = units[unit_type][unit_i];
        int num_cells = 0;

        for (int cell_i = 0; cell_i < 9; cell_i++) {
            if (!grid_cell_has_cand(grid, unit[cell_i], value)) continue;

            if (num_cells < 2) {
                out[num_pairs].cells[num_cells++] = cell_i;
            } else {
                num_cells++;
                break;
            }
        }

        if (num_cells == 2) {
            out[num_pairs].unit = unit_i;
            num_pairs++;
        }
    }

    return num_pairs;
}

static bool find_roof(ConjugatePair pairs[], UnitType unit_type, int roof[],
                      int *floor_unit) {
    Bitset16 walls[2];
    for (int i = 0; i < 2; i++) {
        walls[i] = bitset16_from_arr(pairs[i].cells, 2);
    }

    Bitset16 walls_intersection = bitset16_intersection(walls, 2);
    if (bitset16_len(walls_intersection) != 1) return false;
    *floor_unit = bitset16_first(walls_intersection) - 1;

    for (int i = 0; i < 2; i++) {
        int roof_pos = bitset16_first(walls[i] & ~walls_intersection) - 1;
        roof[i] = cell_from_unit_pos(pairs[i].unit, roof_pos, unit_type);
    }

    return true;
}
