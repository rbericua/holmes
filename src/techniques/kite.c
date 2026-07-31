#include "techniques/kite.h"

#include <stdbool.h>
#include <stdlib.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "techniques/explain.h"
#include "ui/colors.h"
#include "util/dynstr.h"

static int find_conjugate_pairs(Grid *grid, UnitType unit_type, int value,
                                int out[][2]);
static bool find_endpoints(int row_pair[], int col_pair[], int out_endpoints[],
                           int out_link[]);
static int find_removal(Grid *grid, int endpoints[], int value);

bool kite(Grid *grid, Step *step) {
    step->type = TECH_KITE;
    KiteStep *s = &step->as.kite;

    for (int value = 1; value <= 9; value++) {
        int row_pairs[9][2];
        int num_row_pairs = find_conjugate_pairs(grid, UNIT_ROW, value,
                                                 row_pairs);
        if (num_row_pairs == 0) continue;

        int col_pairs[9][2];
        int num_col_pairs = find_conjugate_pairs(grid, UNIT_COL, value,
                                                 col_pairs);
        if (num_col_pairs == 0) continue;

        for (int row_pair_i = 0; row_pair_i < num_row_pairs; row_pair_i++) {
            int *row_pair = row_pairs[row_pair_i];
            if (cells_in_same_box(row_pair, 2)) continue;

            for (int col_pair_i = 0; col_pair_i < num_col_pairs; col_pair_i++) {
                int *col_pair = col_pairs[col_pair_i];
                if (cells_in_same_box(col_pair, 2)) continue;

                if (!find_endpoints(row_pair, col_pair, s->endpoints, s->link))
                    continue;

                s->value = value;
                s->removal_cell = find_removal(grid, s->endpoints, value);
                if (s->removal_cell != -1) return true;
            }
        }
    }

    return false;
}

void kite_apply(Grid *grid, Step *step) {
    KiteStep *s = &step->as.kite;

    grid_cell_remove_cand(grid, s->removal_cell, s->value);
}

void kite_revert(Grid *grid, Step *step) {
    KiteStep *s = &step->as.kite;

    grid_cell_add_cand(grid, s->removal_cell, s->value);
}

void kite_explain(DynStr *buf, Step *step) {
    KiteStep *s = &step->as.kite;

    char *link_str = explain_cells(s->link, 2);
    char *endpoints_str = explain_cells(s->endpoints, 2);

    ds_append(buf, "[2-String Kite] {%d} on %s through %s\n", s->value,
              endpoints_str, link_str);

    free(link_str);
    free(endpoints_str);

    char *removal_msg = explain_value_removal(s->removal_cell, s->value);
    ds_append(buf, "%s\n", removal_msg);
    free(removal_msg);
}

void kite_colorise(ColorPair colors[81][9], Step *step) {
    KiteStep *s = &step->as.kite;

    for (int i = 0; i < 2; i++) {
        colors[s->link[i]][s->value - 1] = CP_SPECIAL1;
        colors[s->endpoints[i]][s->value - 1] = CP_TRIGGER;
    }

    colors[s->removal_cell][s->value - 1] = CP_REMOVAL;
}

static int find_conjugate_pairs(Grid *grid, UnitType unit_type, int value,
                                int out[][2]) {
    int num_pairs = 0;

    for (int unit_i = 0; unit_i < 9; unit_i++) {
        int *unit = units[unit_type][unit_i];
        int num_cells = 0;

        for (int cell_i = 0; cell_i < 9; cell_i++) {
            if (!grid_cell_has_cand(grid, unit[cell_i], value)) continue;

            if (num_cells < 2) {
                out[num_pairs][num_cells++] = cell_from_unit_pos(unit_i, cell_i,
                                                                 unit_type);
            } else {
                num_cells++;
                break;
            }
        }

        if (num_cells == 2) {
            num_pairs++;
        }
    }

    return num_pairs;
}

static bool find_endpoints(int row_pair[], int col_pair[], int out_endpoints[],
                           int out_link[]) {
    for (int i = 0; i < 2; i++) {
        int row_cell = row_pair[i];
        for (int j = 0; j < 2; j++) {
            int col_cell = col_pair[j];

            if (row_cell == col_cell
                || cell_box(row_cell) != cell_box(col_cell))
                continue;

            out_endpoints[0] = row_pair[(i + 1) % 2];
            out_endpoints[1] = col_pair[(j + 1) % 2];
            out_link[0] = row_cell;
            out_link[1] = col_cell;

            return true;
        }
    }

    return false;
}

static int find_removal(Grid *grid, int endpoints[], int value) {
    for (int line_type = 0; line_type < 2; line_type++) {
        int line1 = cell_unit(endpoints[0], line_type);
        int line2 = cell_unit(endpoints[1], other_line(line_type));
        int corner = cell_from_unit_pos(line1, line2, line_type);
        if (grid_cell_has_cand(grid, corner, value)) {
            return corner;
        }
    }
    return -1;
}
