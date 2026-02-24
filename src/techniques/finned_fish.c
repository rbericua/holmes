#include "techniques/finned_fish.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/combinations.h"
#include "techniques/explain.h"

// There can be at most two fins in a line and they have to be in the same box
// Otherwise, no cell in the cover sets will see all the fins
typedef struct {
    int unit_idx;
    int cell_idxs[MAX_FINNED_FISH_SIZE + 2];
    int num_cells;
} BaseSet;

static bool finned_n_fish_unit(Grid *grid, Cell *units[9][9], Step *step,
                               int size, UnitType unit_type);
static int find_base_sets(Cell *units[9][9], int size, int value,
                          BaseSet out[9]);
static int find_cover_idxs(BaseSet base_sets[], int size, int out[]);
static void find_cover_sets(BaseSet base_sets[], int size, int cover_idxs[],
                            int num_covers, BaseSet out[]);
static int find_fins(int fin_units[], int num_fin_units, BaseSet cover_sets[],
                     int num_covers, UnitType base_type, int out[]);
static void find_actual_covers(int cover_idxs[], int num_covers, int fins[],
                               int num_fins, int out[]);
static int find_removals(Cell *units[9][9], int base_idxs[], int cover_idxs[],
                         int size, int value, int box, int out[]);
static bool arr_contains(int arr[], int len, int val);
static bool in_same_box(int idxs[], int len);
static void arr_copy(int dest[], int src[], int len);

bool finned_x_wing(Grid *grid, Step *step) {
    step->tech = TECH_FINNED_X_WING;

    if (finned_n_fish_unit(grid, grid->rows, step, 2, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, grid->cols, step, 2, UNIT_COL)) return true;
    return false;
}

bool finned_swordfish(Grid *grid, Step *step) {
    step->tech = TECH_FINNED_SWORDFISH;

    if (finned_n_fish_unit(grid, grid->rows, step, 3, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, grid->cols, step, 3, UNIT_COL)) return true;
    return false;
}

bool finned_jellyfish(Grid *grid, Step *step) {
    step->tech = TECH_FINNED_JELLYFISH;

    if (finned_n_fish_unit(grid, grid->rows, step, 4, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, grid->cols, step, 4, UNIT_COL)) return true;
    return false;
}

void finned_fish_apply(Grid *grid, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int i = 0; i < s->num_removals; i++) {
        cell_remove_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void finned_fish_revert(Grid *grid, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int i = 0; i < s->num_removals; i++) {
        cell_add_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void finned_fish_explain(DynStr *ds, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    char *fish_name = FISH_NAME_FROM_SIZE(s->size);
    char *base_str = UNIT_TO_STR_PLURAL(s->unit_type);
    char *cover_str = UNIT_TO_STR_PLURAL(s->unit_type == UNIT_ROW ? UNIT_COL
                                                                  : UNIT_ROW);
    ds_appendf(ds, "[Finned %s (%s ", fish_name, base_str);
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
    ds_appendf(ds, "] {%d} with fin%s on ", s->value,
               s->num_fins > 1 ? "s" : "");
    print_idxs(ds, s->fin_idxs, s->num_fins);
    ds_append(ds, ":\n");
    for (int i = 0; i < s->num_removals; i++) {
        int row = ROW_FROM_IDX(s->removal_idxs[i]);
        int col = COL_FROM_IDX(s->removal_idxs[i]);

        ds_appendf(ds, "- Removed {%d} from r%dc%d\n", s->value, row + 1,
                   col + 1);
    }
}

void finned_fish_colorise(ColorPair colors[81][9], Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

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
    for (int i = 0; i < s->num_fins; i++) {
        int idx = s->fin_idxs[i];
        colors[idx][s->value - 1] = CP_SPECIAL;
    }
}

static bool finned_n_fish_unit(Grid *grid, Cell *units[9][9], Step *step,
                               int size, UnitType unit_type) {
    FinnedFishStep *s = &step->as.finned_fish;
    s->unit_type = unit_type;

    for (int value = 1; value <= 9; value++) {
        BaseSet base_sets[9];
        int num_base_sets = find_base_sets(units, size, value, base_sets);

        if (num_base_sets < size) continue;

        int num_base_combs;
        BaseSet **base_combs = generate_combinations(
            base_sets, num_base_sets, size, sizeof(BaseSet), &num_base_combs);

        for (int base_comb_i = 0; base_comb_i < num_base_combs; base_comb_i++) {
            BaseSet *base_comb = base_combs[base_comb_i];

            int cover_idxs[MAX_FINNED_FISH_SIZE + 2];
            int num_covers = find_cover_idxs(base_comb, size, cover_idxs);

            if (num_covers <= size) continue;

            BaseSet cover_sets[MAX_FINNED_FISH_SIZE + 2];
            find_cover_sets(base_comb, size, cover_idxs, num_covers,
                            cover_sets);

            int num_fin_units = num_covers - size;
            int num_fin_unit_combs;
            int **fin_unit_combs = generate_combinations(
                cover_idxs, num_covers, num_fin_units, sizeof(int),
                &num_fin_unit_combs);

            for (int fin_unit_comb_i = 0; fin_unit_comb_i < num_fin_unit_combs;
                 fin_unit_comb_i++) {
                int *fin_units = fin_unit_combs[fin_unit_comb_i];

                // Don't know the maximum number this could be
                int fins[81];
                int num_fins = find_fins(fin_units, num_fin_units, cover_sets,
                                         num_covers, unit_type, fins);

                if (!in_same_box(fins, num_fins)) continue;
                int removal_box = BOX_FROM_IDX(fins[0]);

                for (int i = 0; i < size; i++) {
                    s->base_idxs[i] = base_comb[i].unit_idx;
                }
                find_actual_covers(cover_idxs, num_covers, fin_units,
                                   num_fin_units, s->cover_idxs);
                s->size = size;
                s->value = value;
                s->num_removals = find_removals(
                    unit_type == UNIT_ROW ? grid->cols : grid->rows,
                    s->base_idxs, s->cover_idxs, size, value, removal_box,
                    s->removal_idxs);
                arr_copy(s->fin_idxs, fins, num_fins);
                s->num_fins = num_fins;

                if (s->num_removals == 0) continue;

                free_combinations(base_combs);
                free_combinations(fin_unit_combs);

                return true;
            }

            free_combinations(fin_unit_combs);
        }

        free_combinations(base_combs);
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
            if (cell_has_cand(cell, value) && num_cells < size + 2) {
                out[num_sets].cell_idxs[num_cells++] = cell_i;
            } else if (has_value) {
                num_cells++;
                break;
            }
        }

        if (num_cells > 0 && num_cells <= size + 2) {
            out[num_sets++].num_cells = num_cells;
        }
    }

    return num_sets;
}

// Since BaseSet.cell_idxs contains numbers from 0 to 8, it's necessary to
// add 1 to each index to make them compatible with CandSet
static int find_cover_idxs(BaseSet base_sets[], int size, int out[]) {
    CandSet sets[MAX_FINNED_FISH_SIZE + 2];

    for (int i = 0; i < size; i++) {
        int adjusted_idxs[MAX_FINNED_FISH_SIZE + 2];
        for (int j = 0; j < base_sets[i].num_cells; j++) {
            adjusted_idxs[j] = base_sets[i].cell_idxs[j] + 1;
        }
        sets[i] = cand_set_from_arr(adjusted_idxs, base_sets[i].num_cells);
    }

    CandSet covers = cand_set_union_from_arr(sets, size);

    if (covers.len > size + 2) return -1;

    cand_set_to_arr(covers, out);
    for (int i = 0; i < covers.len; i++) {
        out[i]--;
    }

    return covers.len;
}

static void find_cover_sets(BaseSet base_sets[], int size, int cover_idxs[],
                            int num_covers, BaseSet out[]) {
    for (int i = 0; i < num_covers; i++) {
        int cover_len = 0;
        out[i].unit_idx = cover_idxs[i];
        for (int j = 0; j < size; j++) {
            BaseSet base = base_sets[j];
            if (arr_contains(base.cell_idxs, base.num_cells, cover_idxs[i])) {
                out[i].cell_idxs[cover_len++] = base.unit_idx;
            }
        }
        out[i].num_cells = cover_len;
    }
}

static int find_fins(int fin_units[], int num_fin_units, BaseSet cover_sets[],
                     int num_covers, UnitType base_type, int out[]) {
    int box_first;
    int count = 0;
    for (int i = 0; i < num_fin_units; i++) {
        int fin_unit = fin_units[i];
        for (int j = 0; j < num_covers; j++) {
            BaseSet cover = cover_sets[j];
            if (fin_unit != cover.unit_idx) continue;
            for (int k = 0; k < cover.num_cells; k++) {
                int base_idx = cover.cell_idxs[k];
                out[count++] = base_type == UNIT_ROW
                                   ? IDX_FROM_ROW_COL(base_idx, fin_unit)
                                   : IDX_FROM_ROW_COL(fin_unit, base_idx);
                if (count == 1) {
                    box_first = BOX_FROM_IDX(out[0]);
                } else if (BOX_FROM_IDX(out[count - 1] != box_first)) {
                    return -1;
                }
            }
            break;
        }
    }
    return count;
}

static void find_actual_covers(int cover_idxs[], int num_covers, int fins[],
                               int num_fins, int out[]) {
    int count = 0;
    for (int i = 0; i < num_covers; i++) {
        if (!arr_contains(fins, num_fins, cover_idxs[i])) {
            out[count++] = cover_idxs[i];
        }
    }
}

static int find_removals(Cell *units[9][9], int base_idxs[], int cover_idxs[],
                         int size, int value, int box, int out[]) {
    int count = 0;
    // int base_i = 0;
    for (int cell_i = 0; cell_i < 9; cell_i++) {
        if (arr_contains(base_idxs, size, cell_i)) continue;
        // if (cell_i == base_idxs[base_i]) {
        //     base_i++;
        //     continue;
        // }

        for (int cover_i = 0; cover_i < size; cover_i++) {
            int cover_idx = cover_idxs[cover_i];
            Cell *cell = units[cover_idx][cell_i];
            if (cell_has_cand(cell, value) && cell->box == box) {
                out[count++] = cell_idx(cell);
            }
        }
    }

    return count;
}

static bool arr_contains(int arr[], int len, int val) {
    for (int i = 0; i < len; i++) {
        if (arr[i] == val) return true;
    }
    return false;
}

static bool in_same_box(int idxs[], int len) {
    int box_first = BOX_FROM_IDX(idxs[0]);
    for (int i = 1; i < len; i++) {
        if (BOX_FROM_IDX(idxs[i]) != box_first) return false;
    }
    return true;
}

static void arr_copy(int dest[], int src[], int len) {
    for (int i = 0; i < len; i++) {
        dest[i] = src[i];
    }
}
