#include "techniques/simple_coloring.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "techniques/explain.h"
#include "ui/colors.h"
#include "ui/pipes.h"
#include "util/dynarr.h"
#include "util/dynstr.h"

static int other_color(int color) {
    return color == 1 ? 2 : 1;
}

static bool find_links(Grid *grid, int value, int links[][3]);
static int find_first_linked(int links[][3]);
static void paint_and_extract_links(int links[][3], int cell, int curr_color,
                                    int colors[], int out_links[][2],
                                    int *num_links);
static int extract_colors(int colors[], int out_cells[], int out_colors[]);
static bool find_removals(SimpleColoringStep *s, Grid *grid, int value,
                          int colors[]);

bool simple_coloring(Grid *grid, Step *step) {
    step->type = TECH_SIMPLE_COLORING;
    SimpleColoringStep *s = &step->as.simple_coloring;

    for (int value = 1; value <= 9; value++) {
        int links[81][3];
        for (int i = 0; i < 81; i++) {
            for (int j = 0; j < 3; j++) {
                links[i][j] = -1;
            }
        }
        find_links(grid, value, links);

        int first;
        while ((first = find_first_linked(links)) != -1) {
            int colors[81] = {0};
            s->num_links = 0;
            paint_and_extract_links(links, first, 1, colors, s->links,
                                    &s->num_links);
            s->chain_len = extract_colors(colors, s->cells, s->colors);
            s->value = value;

            if (find_removals(s, grid, value, colors)) return true;
        }
    }

    return false;
}

void simple_coloring_apply(Grid *grid, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    switch (s->rule) {
    case SC_TWICE_IN_UNIT:
        for (int i = 0; i < s->chain_len; i++) {
            if (s->colors[i] == s->twice_in_unit.color) {
                grid_cell_remove_cand(grid, s->cells[i], s->value);
            }
        }
        break;
    case SC_BOTH_SEEN:
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            grid_cell_remove_cand(grid, s->both_seen.removal_cells[i],
                                  s->value);
        }
        break;
    }
}

void simple_coloring_revert(Grid *grid, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    switch (s->rule) {
    case SC_TWICE_IN_UNIT:
        for (int i = 0; i < s->chain_len; i++) {
            if (s->colors[i] == s->twice_in_unit.color) {
                grid_cell_add_cand(grid, s->cells[i], s->value);
            }
        }
        break;
    case SC_BOTH_SEEN:
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            grid_cell_add_cand(grid, s->both_seen.removal_cells[i], s->value);
        }
        break;
    }
}

void simple_coloring_explain(DynStr *buf, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    switch (s->rule) {
    case SC_TWICE_IN_UNIT: {
        char *cells_str = explain_cells(s->twice_in_unit.cells, 2);

        ds_append(buf,
                  "[Simple Coloring (Twice in Unit)] %s see each "
                  "other and have the same color\n",
                  cells_str);
        for (int i = 0; i < s->chain_len; i++) {
            if (s->colors[i] == s->twice_in_unit.color) {
                char *removal_msg = explain_value_removal(s->cells[i],
                                                          s->value);
                ds_append(buf, "%s\n", removal_msg);
                free(removal_msg);
            }
        }

        free(cells_str);
    } break;
    case SC_BOTH_SEEN: {
        ds_append(buf, "[Simple Coloring (Both Seen)]\n");
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            char *removal_msg = explain_value_removal(
                s->both_seen.removal_cells[i], s->value);
            ds_append(buf, "%s\n", removal_msg);
            free(removal_msg);
        }
        break;
    }
    }
}

void simple_coloring_colorise(ColorPair colors[81][9], Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    for (int i = 0; i < s->chain_len; i++) {
        int color;
        if (s->rule == SC_TWICE_IN_UNIT
            && s->colors[i] == s->twice_in_unit.color) {
            color = CP_REMOVAL;
        } else {
            color = s->colors[i] == 1 ? CP_SPECIAL1 : CP_SPECIAL2;
        }
        colors[s->cells[i]][s->value - 1] = color;
    }
    if (s->rule == SC_BOTH_SEEN) {
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            colors[s->both_seen.removal_cells[i]][s->value - 1] = CP_REMOVAL;
        }
    }
}

void simple_coloring_pipes(Step *step, Pipes *pipes) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    for (int i = 0; i < s->num_links; i++) {
        da_append(pipes, pipe_create(s->links[i][0], s->value, s->links[i][1],
                                     s->value));
    }
}

static bool find_links(Grid *grid, int value, int links[][3]) {
    bool found = false;
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];
            int pair[9];
            if (grid_region_with_cand(grid, unit, 9, value, pair) != 2)
                continue;
            links[pair[0]][unit_type] = pair[1];
            links[pair[1]][unit_type] = pair[0];
            found = true;
        }
    }
    return found;
}

static int find_first_linked(int links[][3]) {
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 3; j++) {
            if (links[i][j] != -1) return i;
        }
    }
    return -1;
}

static void paint_and_extract_links(int links[][3], int cell, int curr_color,
                                    int colors[], int out_links[][2],
                                    int *num_links) {
    colors[cell] = curr_color;
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        int other_cell = links[cell][unit_type];
        if (other_cell != -1) {
            links[cell][unit_type] = -1;
            links[other_cell][unit_type] = -1;

            out_links[*num_links][0] = cell;
            out_links[(*num_links)++][1] = other_cell;
            paint_and_extract_links(links, other_cell, other_color(curr_color),
                                    colors, out_links, num_links);
        }
    }
}

static int extract_colors(int colors[], int out_cells[], int out_colors[]) {
    int count = 0;
    for (int i = 0; i < 81; i++) {
        if (colors[i] != 0) {
            out_cells[count] = i;
            out_colors[count++] = colors[i];
        }
    }
    return count;
}

static bool find_removals(SimpleColoringStep *s, Grid *grid, int value,
                          int colors[]) {
    s->rule = SC_BOTH_SEEN;
    s->both_seen.num_removals = 0;

    for (int cell_i = 0; cell_i < 80; cell_i++) {
        if (colors[cell_i] == 0) {
            if (!grid_cell_has_cand(grid, cell_i, value)) continue;
            bool seen[3] = {0};
            for (int peer_i = 0; peer_i < NUM_PEERS; peer_i++) {
                int peer = peers[cell_i][peer_i];
                seen[colors[peer]] = true;
            }
            if (seen[1] && seen[2]) {
                s->both_seen.removal_cells[s->both_seen.num_removals++] =
                    cell_i;
            }
            continue;
        }

        for (int cell_j = cell_i + 1; cell_j < 81; cell_j++) {
            int cells[] = {cell_i, cell_j};
            if (colors[cell_j] == colors[cell_i]
                && cells_are_peers(cells[0], cells[1])) {
                s->rule = SC_TWICE_IN_UNIT;
                memcpy(s->twice_in_unit.cells, cells, 2 * sizeof(int));
                s->twice_in_unit.color = colors[cell_i];
                return true;
            }
        }
    }

    return s->both_seen.num_removals > 0;
}
