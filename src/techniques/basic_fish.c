#include "techniques/basic_fish.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/combinations.h"
#include "techniques/explain.h"

typedef struct {
    int unit_idx;
    int cell_idxs[MAX_BASIC_FISH_SIZE];
    int num_cells;
} BaseSet;

static bool n_fish_unit(Grid *grid, Cell *units[9][9], Step *step, int size,
                        UnitType unit_type);
static int find_base_sets(Cell *units[9][9], int value, int size,
                          BaseSet out[9]);
static bool is_valid_fish(BaseSet base_sets[MAX_BASIC_FISH_SIZE], int size,
                          int out_covers[MAX_BASIC_FISH_SIZE]);
static int find_removals(Cell *units[9][9], int base_idxs[MAX_BASIC_FISH_SIZE],
                         int cover_idxs[MAX_BASIC_FISH_SIZE], int size,
                         int value, int out[MAX_BASIC_FISH_REMOVALS]);

bool x_wing(Grid *grid, Step *step) {
    step->tech = TECH_X_WING;

    if (n_fish_unit(grid, grid->rows, step, 2, UNIT_ROW)) return true;
    if (n_fish_unit(grid, grid->cols, step, 2, UNIT_COL)) return true;
    return false;
}

bool swordfish(Grid *grid, Step *step) {
    step->tech = TECH_SWORDFISH;

    if (n_fish_unit(grid, grid->rows, step, 3, UNIT_ROW)) return true;
    if (n_fish_unit(grid, grid->cols, step, 3, UNIT_COL)) return true;
    return false;
}

bool jellyfish(Grid *grid, Step *step) {
    step->tech = TECH_JELLYFISH;

    if (n_fish_unit(grid, grid->rows, step, 4, UNIT_ROW)) return true;
    if (n_fish_unit(grid, grid->cols, step, 4, UNIT_COL)) return true;
    return false;
}

void basic_fish_apply(Grid *grid, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->num_removals; i++) {
        cell_remove_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void basic_fish_revert(Grid *grid, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->num_removals; i++) {
        cell_add_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void basic_fish_explain(DynStr *ds, Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    char *fish_name = FISH_NAME_FROM_SIZE(s->size);
    char *base_str = UNIT_TO_STR_PLURAL(s->unit_type);
    char *cover_str = UNIT_TO_STR_PLURAL(s->unit_type == UNIT_ROW ? UNIT_COL
                                                                  : UNIT_ROW);
    ds_appendf(ds, "[%s (%s ", fish_name, base_str);
    for (int i = 0; i < s->size; i++) {
        ds_appendf(ds, "%d", s->base_idxs[i] + 1);
        if (i < s->size - 1) {
            ds_append(ds, ", ");
        }
    }
    ds_appendf(ds, " -> %s ", cover_str);
    for (int i = 0; i < s->size; i++) {
        ds_appendf(ds, "%d", s->cover_idxs[i] + 1);
        if (i < s->size - 1) {
            ds_append(ds, ", ");
        }
    }
    ds_appendf(ds, "] {%d}:\n", s->value);
    for (int i = 0; i < s->num_removals; i++) {
        int row = ROW_FROM_IDX(s->removal_idxs[i]);
        int col = COL_FROM_IDX(s->removal_idxs[i]);

        ds_appendf(ds, "- Removed {%d} from r%dc%d\n", s->value, row + 1,
                   col + 1);
    }
}

void basic_fish_colorise(ColorPair colors[81][9], Step *step) {
    BasicFishStep *s = &step->as.basic_fish;

    for (int i = 0; i < s->size; i++) {
        int base = s->base_idxs[i];
        for (int j = 0; j < s->size; j++) {
            int cover = s->cover_idxs[j];
            int idx = s->unit_type == UNIT_ROW ? IDX_FROM_ROW_COL(base, cover)
                                               : IDX_FROM_ROW_COL(cover, base);
            colors[idx][s->value - 1] = CP_TRIGGER;
        }
    }
    for (int i = 0; i < s->num_removals; i++) {
        int idx = s->removal_idxs[i];
        colors[idx][s->value - 1] = CP_REMOVAL;
    }
}

static bool n_fish_unit(Grid *grid, Cell *units[9][9], Step *step, int size,
                        UnitType unit_type) {
    BasicFishStep *s = &step->as.basic_fish;
    s->unit_type = unit_type;

    for (int value = 1; value <= 9; value++) {
        BaseSet base_sets[9];
        int num_base_sets = find_base_sets(units, size, value, base_sets);

        if (num_base_sets < size) continue;

        int num_combs;
        BaseSet **combs = generate_combinations(base_sets, num_base_sets, size,
                                                sizeof(BaseSet), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            BaseSet *comb = combs[comb_i];

            if (!is_valid_fish(comb, size, s->cover_idxs)) continue;

            for (int i = 0; i < size; i++) {
                s->base_idxs[i] = comb[i].unit_idx;
            }
            s->size = size;
            s->value = value;
            s->num_removals = find_removals(
                unit_type == UNIT_ROW ? grid->cols : grid->rows, s->base_idxs,
                s->cover_idxs, size, value, s->removal_idxs);

            if (s->num_removals != 0) {
                free_combinations(combs);
                return true;
            }
        }

        free_combinations(combs);
    }

    return false;
}

static int find_base_sets(Cell *units[9][9], int size, int value,
                          BaseSet out[9]) {
    int num_sets = 0;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        out[num_sets].unit_idx = unit_i;
        int num_cells = 0;

        for (int cell_i = 0; cell_i < 9; cell_i++) {
            Cell *cell = units[unit_i][cell_i];
            bool has_value = cell_has_cand(cell, value);
            if (has_value && num_cells < size) {
                out[num_sets].cell_idxs[num_cells++] = cell_i;
            } else if (has_value) {
                num_cells++;
                break;
            }
        }

        if (num_cells > 0 && num_cells <= size) {
            out[num_sets++].num_cells = num_cells;
        }
    }

    return num_sets;
}

// Since BaseSet.cell_idxs contains numbers from 0 to 8, it's necessary to add 1
// to each index to make them compatible with CandSet
static bool is_valid_fish(BaseSet base_sets[MAX_BASIC_FISH_SIZE], int size,
                          int out_covers[MAX_BASIC_FISH_SIZE]) {
    CandSet sets[MAX_BASIC_FISH_SIZE];

    for (int i = 0; i < size; i++) {
        int adjusted_idxs[MAX_BASIC_FISH_SIZE];
        for (int j = 0; j < base_sets[i].num_cells; j++) {
            adjusted_idxs[j] = base_sets[i].cell_idxs[j] + 1;
            sets[i] = cand_set_from_arr(adjusted_idxs, size);
        }
    }

    CandSet covers = cand_set_union_from_arr(sets, size);

    if (covers.len != size) return false;

    cand_set_to_arr(covers, out_covers);
    for (int i = 0; i < size; i++) {
        out_covers[i]--;
    }

    return true;
}

static int find_removals(Cell *units[9][9], int base_idxs[MAX_BASIC_FISH_SIZE],
                         int cover_idxs[MAX_BASIC_FISH_SIZE], int size,
                         int value, int out[MAX_BASIC_FISH_REMOVALS]) {
    int count = 0;
    int base_i = 0;
    for (int cell_i = 0; cell_i < 9; cell_i++) {
        if (cell_i == base_idxs[base_i]) {
            base_i++;
            continue;
        }

        for (int cover_i = 0; cover_i < size; cover_i++) {
            int cover_idx = cover_idxs[cover_i];
            Cell *cell = units[cover_idx][cell_i];
            if (cell_has_cand(cell, value)) {
                out[count++] = cell_idx(cell);
            }
        }
    }

    return count;
}
