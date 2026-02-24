#include "techniques/hidden_set.h"

#include <stdbool.h>

#include "cand_set.h"
#include "cell.h"
#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "ui.h"
#include "techniques/combinations.h"
#include "techniques/explain.h"

static bool hidden_n_set_unit(Cell *units[9][9], Step *step, int size,
                              UnitType unit_type);

bool hidden_set(Grid *grid, Step *step) {
    if (hidden_pair(grid, step)) return true;
    if (hidden_triple(grid, step)) return true;
    if (hidden_quad(grid, step)) return true;
    return false;
}

bool hidden_pair(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_PAIR;
    step->as.hidden_set.size = 2;

    if (hidden_n_set_unit(grid->rows, step, 2, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 2, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 2, UNIT_BOX)) return true;
    return false;
}

bool hidden_triple(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_TRIPLE;
    step->as.hidden_set.size = 3;

    if (hidden_n_set_unit(grid->rows, step, 3, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 3, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 3, UNIT_BOX)) return true;
    return false;
}

bool hidden_quad(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_QUAD;
    step->as.hidden_set.size = 4;

    if (hidden_n_set_unit(grid->rows, step, 4, UNIT_ROW)) return true;
    if (hidden_n_set_unit(grid->cols, step, 4, UNIT_COL)) return true;
    if (hidden_n_set_unit(grid->boxes, step, 4, UNIT_BOX)) return true;
    return false;
}

void hidden_set_apply(Grid *grid, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->size; i++) {
        grid->cells[s->idxs[i]]->cands = s->cands;
    }
}

void hidden_set_revert(Grid *grid, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->size; i++) {
        cell_add_cands(grid->cells[s->removal_idxs[i]], s->removed_cands[i]);
    }
}

void hidden_set_explain(DynStr *ds, Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    char *unit_str = UNIT_TO_STR(s->unit_type);
    char *set_name = SET_NAME_FROM_SIZE(s->size);

    ds_appendf(ds, "[Hidden %s (%s %d)] ", set_name, unit_str, s->unit_idx + 1);
    print_cand_set(ds, s->cands);
    ds_append(ds, " in ");
    print_idxs(ds, s->idxs, s->size);
    ds_append(ds, ":\n");
    for (int i = 0; i < s->num_removals; i++) {
        int row = ROW_FROM_IDX(s->removal_idxs[i]);
        int col = COL_FROM_IDX(s->removal_idxs[i]);

        ds_append(ds, "- Removed ");
        print_cand_set(ds, s->removed_cands[i]);
        ds_appendf(ds, " from r%dc%d\n", row + 1, col + 1);
    }
}

void hidden_set_colorise(ColorPair colors[81][9], Step *step) {
    HiddenSetStep *s = &step->as.hidden_set;

    for (int i = 0; i < s->size; i++) {
        int idx = s->idxs[i];
        for (int cand = 1; cand <= 9; cand++) {
            colors[idx][cand - 1] = cand_set_has(s->cands, cand) ? CP_TRIGGER
                                                                 : CP_REMOVAL;
        }
    }
}

static bool hidden_n_set_unit(Cell *units[9][9], Step *step, int size,
                              UnitType unit_type) {
    HiddenSetStep *s = &step->as.hidden_set;
    s->unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        int missing_values[9];
        int num_missing_values = cells_missing_values_to_arr(unit, 9,
                                                             missing_values);

        if (num_missing_values < size) continue;

        int num_combs;
        int **combs = generate_combinations(missing_values, num_missing_values,
                                            size, sizeof(int), &num_combs);

        for (int comb_i = 0; comb_i < num_combs; comb_i++) {
            CandSet comb_set = cand_set_from_arr(combs[comb_i], size);

            Cell *possible_cells[9];
            int num_possible_cells = cells_with_cands_some(unit, 9, comb_set,
                                                           possible_cells);

            if (num_possible_cells != size) continue;

            Cell *removal_cells[MAX_HIDDEN_SET_REMOVALS];
            int num_removals = cells_with_removals(
                possible_cells, num_possible_cells,
                cand_set_difference(cand_set_full(), comb_set), removal_cells,
                s->removed_cands);

            if (num_removals == 0) continue;

            cells_idxs(possible_cells, size, s->idxs);
            s->cands = comb_set;
            cells_idxs(removal_cells, num_removals, s->removal_idxs);
            s->num_removals = num_removals;
            s->unit_idx = unit_i;

            free_combinations(combs);

            return true;
        }

        free_combinations(combs);
    }

    return false;
}
