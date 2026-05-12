#include "techniques/finned_fish.h"

#include <stdbool.h>
#include <stdlib.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/explain.h"
#include "util/combinations.h"
#include "util/dynstr.h"

typedef struct {
    int idx;
    int cells[MAX_FINNED_FISH_SIZE + 2];
    int len;
} UnitSubset;

static bool arr_contains(int arr[], int len, int elem) {
    for (int i = 0; i < len; i++) {
        if (arr[i] == elem) return true;
    }
    return false;
}

static int cell_from_base_cover(int base_idx, int cover_idx,
                                UnitType base_type) {
    int row = base_type == UNIT_ROW ? base_idx : cover_idx;
    int col = base_type == UNIT_ROW ? cover_idx : base_idx;
    return cell_from_row_col(row, col);
}

static bool finned_n_fish_unit(Grid *grid, Step *step, int fish_size,
                               UnitType unit_type);
static int find_base_sets(Grid *grid, int fish_size, UnitType unit_type,
                          int value, UnitSubset out[]);
static int find_cover_sets(UnitSubset bases[], int fish_size,
                           UnitSubset out[9]);
static int find_fins(UnitSubset fin_units[], int num_fin_units,
                     UnitType base_unit_type, int out[]);
static void find_actual_covers(UnitSubset covers[], int num_covers,
                               UnitSubset fin_units[], int num_fin_units,
                               int out[]);
static int find_removals(Grid *grid, int fish_size, int value, int base_idxs[],
                         int cover_idxs[], int base_unit_type,
                         int cover_unit_type, int box, int out[]);

bool finned_x_wing(Grid *grid, Step *step) {
    step->type = TECH_FINNED_X_WING;

    if (finned_n_fish_unit(grid, step, 2, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, step, 2, UNIT_COL)) return true;
    return false;
}

bool finned_swordfish(Grid *grid, Step *step) {
    step->type = TECH_FINNED_SWORDFISH;

    if (finned_n_fish_unit(grid, step, 3, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, step, 3, UNIT_COL)) return true;
    return false;
}

bool finned_jellyfish(Grid *grid, Step *step) {
    step->type = TECH_FINNED_JELLYFISH;

    if (finned_n_fish_unit(grid, step, 4, UNIT_ROW)) return true;
    if (finned_n_fish_unit(grid, step, 4, UNIT_COL)) return true;
    return false;
}

void finned_fish_apply(Grid *grid, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->value);
    }
}

void finned_fish_revert(Grid *grid, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void finned_fish_explain(DynStr *buf, Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    char *fish_name = explain_fish_name(s->fish_size);
    char *base_unit_str = explain_unit_name_plural(s->base_unit_type);
    char *base_idxs_str = explain_nums_plus_one(s->base_idxs, s->fish_size);
    char *cover_unit_str = explain_unit_name_plural(s->cover_unit_type);
    char *cover_idxs_str = explain_nums_plus_one(s->cover_idxs, s->fish_size);
    char *fins_str = explain_cells(s->fins, s->num_fins);

    ds_append(buf, "[Finned %s (%s %s -> %s %s)] {%d} with fin%s on %s:\n",
              fish_name, base_unit_str, base_idxs_str, cover_unit_str,
              cover_idxs_str, s->value, s->num_fins > 1 ? "s" : "", fins_str);

    free(fish_name);
    free(base_unit_str);
    free(base_idxs_str);
    free(cover_unit_str);
    free(cover_idxs_str);
    free(fins_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->value);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void finned_fish_colorise(ColorPair colors[81][9], Step *step) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int i = 0; i < s->fish_size; i++) {
        for (int j = 0; j < s->fish_size; j++) {
            int cell = cell_from_base_cover(s->base_idxs[i], s->cover_idxs[j],
                                            s->base_unit_type);
            colors[cell][s->value - 1] = CP_TRIGGER;
        }
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
    for (int i = 0; i < s->num_fins; i++) {
        colors[s->fins[i]][s->value - 1] = CP_SPECIAL;
    }
}

static bool finned_n_fish_unit(Grid *grid, Step *step, int fish_size,
                               UnitType unit_type) {
    FinnedFishStep *s = &step->as.finned_fish;

    for (int value = 1; value <= 9; value++) {
        UnitSubset bases[9];
        int num_bases = find_base_sets(grid, fish_size, unit_type, value,
                                       bases);

        if (num_bases < fish_size) continue;

        int num_base_combs;
        UnitSubset **base_combs = generate_combinations(
            bases, num_bases, fish_size, sizeof(UnitSubset), &num_base_combs);

        for (int base_comb_i = 0; base_comb_i < num_base_combs; base_comb_i++) {
            UnitSubset *base_comb = base_combs[base_comb_i];

            UnitSubset covers[9] = {0};
            int num_covers = find_cover_sets(base_comb, fish_size, covers);
            if (num_covers <= fish_size || num_covers > fish_size + 2) continue;

            int num_fin_units = num_covers - fish_size;
            int num_fin_unit_combs;
            UnitSubset **fin_unit_combs = generate_combinations(
                covers, num_covers, num_fin_units, sizeof(UnitSubset),
                &num_fin_unit_combs);

            for (int fin_unit_comb_i = 0; fin_unit_comb_i < num_fin_unit_combs;
                 fin_unit_comb_i++) {
                UnitSubset *fin_unit_comb = fin_unit_combs[fin_unit_comb_i];

                s->num_fins = find_fins(fin_unit_comb, num_fin_units, unit_type,
                                        s->fins);
                if (s->num_fins == -1
                    || !cells_in_same_box(s->fins, s->num_fins))
                    continue;
                int fin_box = cell_box(s->fins[0]);

                for (int i = 0; i < fish_size; i++) {
                    s->base_idxs[i] = base_comb[i].idx;
                }
                find_actual_covers(covers, num_covers, fin_unit_comb,
                                   num_fin_units, s->cover_idxs);
                s->fish_size = fish_size;
                s->value = value;
                s->base_unit_type = unit_type;
                s->cover_unit_type = other_line(unit_type);
                s->num_removals = find_removals(
                    grid, fish_size, value, s->base_idxs, s->cover_idxs,
                    s->base_unit_type, s->cover_unit_type, fin_box,
                    s->removal_cells);

                if (s->num_removals == 0) continue;

                free_combinations(fin_unit_combs);
                free_combinations(base_combs);

                return true;
            }

            free_combinations(fin_unit_combs);
        }

        free_combinations(base_combs);
    }

    return false;
}

static int find_base_sets(Grid *grid, int fish_size, UnitType unit_type,
                          int value, UnitSubset out[]) {
    int num_sets = 0;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        int *unit = units[unit_type][unit_i];
        int num_cells = 0;

        for (int cell_i = 0; cell_i < 9; cell_i++) {
            if (grid_cell_has_cand(grid, unit[cell_i], value)) {
                if (num_cells < fish_size + 2) {
                    out[num_sets].cells[num_cells++] = cell_i;
                } else {
                    num_cells++;
                    break;
                }
            }
        }

        if (num_cells > 0 && num_cells <= fish_size + 2) {
            out[num_sets].idx = unit_i;
            out[num_sets].len = num_cells;
            num_sets++;
        }
    }

    return num_sets;
}

static int find_cover_sets(UnitSubset bases[], int fish_size,
                           UnitSubset out[9]) {
    for (int base_i = 0; base_i < fish_size; base_i++) {
        UnitSubset base = bases[base_i];
        for (int cell_i = 0; cell_i < base.len; cell_i++) {
            int cell = base.cells[cell_i];
            out[cell].cells[out[cell].len++] = base.idx;
        }
    }

    int num_covers = 0;
    for (int i = 0; i < 9; i++) {
        if (out[i].len > 0) {
            out[i].idx = i;
            out[num_covers++] = out[i];
        }
    }

    return num_covers;
}

static int find_fins(UnitSubset fin_units[], int num_fin_units,
                     UnitType base_unit_type, int out[]) {
    int num_fins = 0;
    for (int i = 0; i < num_fin_units; i++) {
        for (int j = 0; j < fin_units[i].len; j++) {
            if (num_fins == MAX_FINS) return -1;
            out[num_fins++] = cell_from_base_cover(
                fin_units[i].cells[j], fin_units[i].idx, base_unit_type);
        }
    }
    return num_fins;
}

static void find_actual_covers(UnitSubset covers[], int num_covers,
                               UnitSubset fin_units[], int num_fin_units,
                               int out[]) {
    int cover_i = 0;
    int fin_unit_i = 0;
    int count = 0;
    while (cover_i < num_covers && fin_unit_i < num_fin_units) {
        if (covers[cover_i].idx == fin_units[fin_unit_i].idx) {
            fin_unit_i++;
        } else {
            out[count++] = covers[cover_i].idx;
        }
        cover_i++;
    }
    while (cover_i < num_covers) {
        out[count++] = covers[cover_i++].idx;
    }
}

static int find_removals(Grid *grid, int fish_size, int value, int base_idxs[],
                         int cover_idxs[], int base_unit_type,
                         int cover_unit_type, int box, int out[]) {
    int count = 0;
    for (int i = 0; i < 9; i++) {
        int cell = units[UNIT_BOX][box][i];
        if (grid_cell_has_cand(grid, cell, value)
            && arr_contains(cover_idxs, fish_size,
                            cell_unit(cell, cover_unit_type))
            && !arr_contains(base_idxs, fish_size,
                             cell_unit(cell, base_unit_type))) {
            out[count++] = cell;
        }
    }
    return count;
}
