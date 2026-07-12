#include "techniques/basic_fish.h"

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
    int idx;
    int cells[MAX_BASIC_FISH_SIZE];
    int len;
} UnitSubset;

static int cell_from_base_cover(int base_idx, int cover_idx,
                                UnitType base_type) {
    int row = base_type == UNIT_ROW ? base_idx : cover_idx;
    int col = base_type == UNIT_ROW ? cover_idx : base_idx;
    return cell_from_row_col(row, col);
}

static bool basic_n_fish_unit(Grid *grid, Step *step, int size,
                              UnitType unit_type);
static int find_base_sets(Grid *grid, int size, UnitType unit_type, int value,
                          UnitSubset out[]);
static bool find_cover_idxs(UnitSubset base_sets[], int size, int out[]);
static int find_removals(Grid *grid, UnitSubset base_sets[], int cover_idxs[],
                         int size, UnitType unit_type, int value, int out[]);

bool x_wing(Grid *grid, Step *step) {
    step->type = TECH_X_WING;

    if (basic_n_fish_unit(grid, step, 2, UNIT_ROW)) return true;
    if (basic_n_fish_unit(grid, step, 2, UNIT_COL)) return true;
    return false;
}

bool swordfish(Grid *grid, Step *step) {
    step->type = TECH_SWORDFISH;

    if (basic_n_fish_unit(grid, step, 3, UNIT_ROW)) return true;
    if (basic_n_fish_unit(grid, step, 3, UNIT_COL)) return true;
    return false;
}

bool jellyfish(Grid *grid, Step *step) {
    step->type = TECH_JELLYFISH;

    if (basic_n_fish_unit(grid, step, 4, UNIT_ROW)) return true;
    if (basic_n_fish_unit(grid, step, 4, UNIT_COL)) return true;
    return false;
}

void basic_fish_apply(Grid *grid, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->value);
    }
}

void basic_fish_revert(Grid *grid, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void basic_fish_explain(DynStr *buf, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    char *fish_name = explain_fish_name(s->size);
    char *base_unit_str = explain_unit_name_plural(s->base_unit_type);
    char *base_idxs_str = explain_nums_plus_one(s->base_idxs, s->size);
    char *cover_unit_str = explain_unit_name_plural(s->cover_unit_type);
    char *cover_idxs_str = explain_nums_plus_one(s->cover_idxs, s->size);

    ds_append(buf, "[%s (%s %s -> %s %s)] {%d}:\n", fish_name, base_unit_str,
              base_idxs_str, cover_unit_str, cover_idxs_str, s->value);

    free(fish_name);
    free(base_unit_str);
    free(base_idxs_str);
    free(cover_unit_str);
    free(cover_idxs_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->value);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void basic_fish_colorise(ColorPair colors[81][9], Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->size; i++) {
        for (int j = 0; j < s->size; j++) {
            int cell = cell_from_base_cover(s->base_idxs[i], s->cover_idxs[j],
                                            s->base_unit_type);
            colors[cell][s->value - 1] = CP_TRIGGER;
        }
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}

static bool basic_n_fish_unit(Grid *grid, Step *step, int size,
                              UnitType unit_type) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int value = 1; value <= 9; value++) {
        UnitSubset base_sets[9];
        int num_base_sets = find_base_sets(grid, size, unit_type, value,
                                           base_sets);

        if (num_base_sets < size) continue;

        int num_combs;
        UnitSubset **combs = generate_combinations(
            base_sets, num_base_sets, size, sizeof(UnitSubset), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            UnitSubset *comb = combs[comb_i];

            if (!find_cover_idxs(comb, size, s->cover_idxs)) continue;

            s->num_removals = find_removals(grid, comb, s->cover_idxs, size,
                                            unit_type, value, s->removal_cells);
            if (s->num_removals == 0) continue;

            for (int i = 0; i < size; i++) {
                s->base_idxs[i] = comb[i].idx;
            }
            s->size = size;
            s->value = value;
            s->base_unit_type = unit_type;
            s->cover_unit_type = other_line(unit_type);

            free_combinations(combs);

            return true;
        }

        free_combinations(combs);
    }

    return false;
}

static int find_base_sets(Grid *grid, int size, UnitType unit_type, int value,
                          UnitSubset out[]) {
    int num_sets = 0;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        int *unit = units[unit_type][unit_i];
        int num_cells = 0;

        for (int cell_i = 0; cell_i < 9; cell_i++) {
            if (grid_cell_has_cand(grid, unit[cell_i], value)) {
                if (num_cells < size) {
                    out[num_sets].cells[num_cells++] = cell_i;
                } else {
                    num_cells++;
                    break;
                }
            }
        }

        if (num_cells > 0 && num_cells <= size) {
            out[num_sets].idx = unit_i;
            out[num_sets].len = num_cells;
            num_sets++;
        }
    }

    return num_sets;
}

static bool find_cover_idxs(UnitSubset base_sets[], int size, int out[]) {
    Bitset16 base_cells[MAX_BASIC_FISH_SIZE];
    for (int i = 0; i < size; i++) {
        base_cells[i] = bitset16_from_arr(base_sets[i].cells, base_sets[i].len);
    }

    Bitset16 covers = bitset16_union(base_cells, size);
    if (bitset16_len(covers) != size) return false;

    bitset16_to_arr(covers, out);
    return true;
}

static int find_removals(Grid *grid, UnitSubset base_sets[], int cover_idxs[],
                         int size, UnitType unit_type, int value, int out[]) {
    int count = 0;
    int base_idx = 0;

    for (int cell_i = 0; cell_i < 9; cell_i++) {
        if (base_idx < size && cell_i == base_sets[base_idx].idx) {
            base_idx++;
            continue;
        }

        for (int cover_i = 0; cover_i < size; cover_i++) {
            int cover_idx = cover_idxs[cover_i];
            int cell = cell_from_base_cover(cell_i, cover_idx, unit_type);
            if (grid_cell_has_cand(grid, cell, value)) {
                out[count++] = cell;
            }
        }
    }

    return count;
}
