#include "techniques/hidden_single.h"

#include <stdbool.h>

#include "cell.h"
#include "grid.h"
#include "step.h"
#include "ui.h"

#define UNIT_TO_STR(u) \
    ((u) == UNIT_ROW ? "Row" : (u) == UNIT_COL ? "Column" : "Box")

static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *step,
                               UnitType unit_type);

bool hidden_single(Grid *grid, Step *step) {
    step->tech = TECH_HIDDEN_SINGLE;

    if (hidden_single_unit(grid, grid->rows, step, UNIT_ROW)) return true;
    if (hidden_single_unit(grid, grid->cols, step, UNIT_COL)) return true;
    if (hidden_single_unit(grid, grid->boxes, step, UNIT_BOX)) return true;
    return false;
}

void hidden_single_apply(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    grid_fill_cell(grid, grid->cells[s->idx], s->value);
}

void hidden_single_revert(Grid *grid, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    grid->cells[s->idx]->value = 0;
    grid->cells[s->idx]->cands = s->old_cands;
    grid->empty_cells++;

    for (int i = 0; i < s->num_removals; i++) {
        cell_add_cand(grid->cells[s->removal_idxs[i]], s->value);
    }
}

void hidden_single_explain(Ui *ui, Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    int row = ROW_FROM_IDX(s->idx);
    int col = COL_FROM_IDX(s->idx);
    char *unit_str = UNIT_TO_STR(s->unit_type);

    ui_print_message(ui, false, false,
                     "[Hidden Single (%s %d)] Set r%dc%d to %d\n", unit_str,
                     s->unit_idx + 1, row + 1, col + 1, s->value);
}

void hidden_single_colorise(ColorPair colors[81][9], Step *step) {
    HiddenSingleStep *s = &step->as.hidden_single;

    for (int cand = 1; cand <= 9; cand++) {
        colors[s->idx][cand - 1] = cand == s->value ? CP_TRIGGER : CP_REMOVAL;
    }
    for (int i = 0; i < s->num_removals; i++) {
        int removal_idx = s->removal_idxs[i];
        colors[removal_idx][s->value - 1] = CP_REMOVAL;
    }
}

static bool hidden_single_unit(Grid *grid, Cell *units[9][9], Step *step,
                               UnitType unit_type) {
    HiddenSingleStep *s = &step->as.hidden_single;
    s->unit_type = unit_type;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        Cell **unit = units[unit_i];

        int missing_values[9];
        int num_missing_values = cells_missing_values_to_arr(unit, 9,
                                                             missing_values);

        for (int value_i = 0; value_i < num_missing_values; value_i++) {
            int value = missing_values[value_i];

            Cell *possible_cells[9];
            int num_possible_cells = cells_with_cand(unit, 9, value,
                                                     possible_cells);

            if (num_possible_cells != 1) continue;

            int idx = cell_idx(possible_cells[0]);

            Cell *removal_cells[MAX_HIDDEN_SINGLE_REMOVALS];
            int num_removals = cells_with_cand(grid->peers[idx], NUM_PEERS,
                                               value, removal_cells);

            s->idx = idx;
            s->value = value;
            cells_idxs(removal_cells, num_removals,
                       step->as.naked_single.removal_idxs);
            s->num_removals = num_removals;
            s->old_cands = possible_cells[0]->cands;
            s->unit_idx = unit_i;

            return true;
        }
    }

    return false;
}
