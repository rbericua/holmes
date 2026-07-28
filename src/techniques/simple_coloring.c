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

static void explain_color_removals(DynStr *buf, SimpleColoringStep *s);
static bool is_link_repeated(int links[][3], int cell1, int cell2);
static bool find_links(Grid *grid, int value, int links[][3]);
static int find_first_linked(int links[][3]);
static void paint_and_extract_links(int links[][3], int cell, int curr_color,
                                    int colors[], int out_links[][2],
                                    int *num_links);
static int extract_chain(int colors[], SimpleColoringNode chain[]);
static int cells_emptied_by_color(int cells[], int num_cells,
                                  int seen_color[][2]);
static bool check_twice_in_unit(int colors[], int seen_color[][2],
                                SimpleColoringStep *s);
static bool check_emptied_unit(Grid *grid, int seen_color[][2],
                               SimpleColoringStep *s);
static bool check_both_seen(Grid *grid, int colors[], int seen_color[][2],
                            SimpleColoringStep *s);
static bool find_removals(Grid *grid, int colors[], SimpleColoringStep *s);

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
        if (!find_links(grid, value, links)) continue;

        int first;
        while ((first = find_first_linked(links)) != -1) {
            int colors[81] = {0};
            s->num_links = 0;
            paint_and_extract_links(links, first, 1, colors, s->links,
                                    &s->num_links);
            s->chain_len = extract_chain(colors, s->chain);
            s->value = value;

            if (find_removals(grid, colors, s)) return true;
        }
    }

    return false;
}

void simple_coloring_apply(Grid *grid, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    if (s->rule == SC_BOTH_SEEN) {
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            grid_cell_remove_cand(grid, s->both_seen.removal_cells[i],
                                  s->value);
        }
    } else {
        for (int i = 0; i < s->chain_len; i++) {
            if (s->chain[i].color == s->removal_color) {
                grid_cell_remove_cand(grid, s->chain[i].cell, s->value);
            }
        }
    }
}

void simple_coloring_revert(Grid *grid, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    if (s->rule == SC_BOTH_SEEN) {
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            grid_cell_add_cand(grid, s->both_seen.removal_cells[i], s->value);
        }
    } else {
        for (int i = 0; i < s->chain_len; i++) {
            if (s->chain[i].color == s->removal_color) {
                grid_cell_add_cand(grid, s->chain[i].cell, s->value);
            }
        }
    }
}

void simple_coloring_explain(DynStr *buf, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    switch (s->rule) {
    case SC_TWICE_IN_UNIT: {
        char *cells_str = explain_cells(s->twice_in_unit.cells, 2);

        ds_append(buf,
                  "[Simple Coloring (Twice in Unit)] %s see each other and "
                  "have the same color\n",
                  cells_str);

        free(cells_str);

        explain_color_removals(buf, s);
    } break;
    case SC_BOTH_SEEN: {
        ds_append(buf, "[Simple Coloring (Both Seen)]\n");
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            char *removal_msg = explain_value_removal(
                s->both_seen.removal_cells[i], s->value);
            ds_append(buf, "%s\n", removal_msg);
            free(removal_msg);
        }
    } break;
    case SC_EMPTIED_UNIT: {
        char *unit_str = explain_unit_name(s->emptied_unit.unit_type);

        ds_append(buf,
                  "[Simple Coloring (Emptied Unit)] Every %d in %s %d sees the "
                  "same color\n",
                  s->value, unit_str, s->emptied_unit.unit_idx + 1);

        free(unit_str);

        explain_color_removals(buf, s);
    } break;
    }
}

void simple_coloring_colorise(ColorPair colors[81][9], Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    for (int i = 0; i < s->chain_len; i++) {
        int color;
        if (s->chain[i].color == s->removal_color) {
            color = CP_REMOVAL;
        } else {
            color = s->chain[i].color == 1 ? CP_SPECIAL1 : CP_SPECIAL2;
        }
        colors[s->chain[i].cell][s->value - 1] = color;
    }

    switch (s->rule) {
    case SC_BOTH_SEEN:
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            colors[s->both_seen.removal_cells[i]][s->value - 1] = CP_REMOVAL;
        }
        break;
    case SC_EMPTIED_UNIT:
        for (int i = 0; i < s->emptied_unit.num_emptied_cells; i++) {
            colors[s->emptied_unit.emptied_cells[i]][s->value - 1] = CP_TRIGGER;
        }
        break;
    default: break;
    }
}

void simple_coloring_pipes(Pipes *pipes, Step *step) {
    SimpleColoringStep *s = &step->as.simple_coloring;

    for (int i = 0; i < s->num_links; i++) {
        da_append(pipes, pipe_create(s->links[i][0], s->value, s->links[i][1],
                                     s->value));
    }
}

static void explain_color_removals(DynStr *buf, SimpleColoringStep *s) {
    for (int i = 0; i < s->chain_len; i++) {
        if (s->chain[i].color == s->removal_color) {
            char *removal_msg = explain_value_removal(s->chain[i].cell,
                                                      s->value);
            ds_append(buf, "%s\n", removal_msg);
            free(removal_msg);
        }
    }
}

static bool is_link_repeated(int links[][3], int cell1, int cell2) {
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        if (links[cell1][unit_type] == cell2) return true;
    }
    return false;
}

static bool find_links(Grid *grid, int value, int links[][3]) {
    bool found = false;
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int *unit = units[unit_type][unit_i];
            int pair[9];
            if (grid_region_with_cand(grid, unit, 9, value, pair) != 2)
                continue;
            if (!is_link_repeated(links, pair[0], pair[1])) {
                links[pair[0]][unit_type] = pair[1];
                links[pair[1]][unit_type] = pair[0];
                found = true;
            }
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

static int extract_chain(int colors[], SimpleColoringNode chain[]) {
    int count = 0;
    for (int i = 0; i < 81; i++) {
        if (colors[i] != 0) {
            chain[count++] = (SimpleColoringNode){
                .cell = i,
                .color = colors[i],
            };
        }
    }
    return count;
}

static int cells_emptied_by_color(int cells[], int num_cells,
                                  int seen_color[][2]) {
    bool emptied_by[] = {true, true};
    for (int cell_i = 0; cell_i < num_cells; cell_i++) {
        int cell = cells[cell_i];
        for (int color = 0; color < 2; color++) {
            if (seen_color[cell][color] == -1) {
                emptied_by[color] = false;
            }
        }
    }
    for (int color_i = 0; color_i < 2; color_i++) {
        if (emptied_by[color_i]) return color_i + 1;
    }
    return 0;
}

static bool check_twice_in_unit(int colors[], int seen_color[][2],
                                SimpleColoringStep *s) {
    for (int cell = 0; cell < 81; cell++) {
        if (seen_color[cell][0] == -1 || seen_color[cell][1] == -1) continue;

        int color = colors[cell];
        if (color != 0) {
            s->rule = SC_TWICE_IN_UNIT;
            s->twice_in_unit.cells[0] = cell;
            s->twice_in_unit.cells[1] = seen_color[cell][color - 1];
            s->removal_color = color;
            return true;
        }
    }
    return false;
}

static bool check_emptied_unit(Grid *grid, int seen_color[][2],
                               SimpleColoringStep *s) {
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            int cells_to_empty[9];
            int num_cells_to_empty = grid_region_with_cand(
                grid, units[unit_type][unit_i], 9, s->value, cells_to_empty);
            if (num_cells_to_empty == 0) continue;

            int emptied_by = cells_emptied_by_color(
                cells_to_empty, num_cells_to_empty, seen_color);

            if (emptied_by != 0) {
                s->rule = SC_EMPTIED_UNIT;
                s->emptied_unit.unit_idx = unit_i;
                s->emptied_unit.unit_type = unit_type;
                s->emptied_unit.unit_type = unit_type;
                memcpy(s->emptied_unit.emptied_cells, cells_to_empty,
                       num_cells_to_empty * sizeof(int));
                s->emptied_unit.num_emptied_cells = num_cells_to_empty;
                s->removal_color = emptied_by;
                return true;
            }
        }
    }
    return false;
}

static bool check_both_seen(Grid *grid, int colors[], int seen_color[][2],
                            SimpleColoringStep *s) {
    s->both_seen.num_removals = 0;
    for (int cell = 0; cell < 81; cell++) {
        if (seen_color[cell][0] == -1 || seen_color[cell][1] == -1) continue;

        int color = colors[cell];
        if (color == 0 && grid_cell_has_cand(grid, cell, s->value)) {
            s->both_seen.removal_cells[s->both_seen.num_removals++] = cell;
        }
    }
    if (s->both_seen.num_removals > 0) {
        s->rule = SC_BOTH_SEEN;
        s->removal_color = -1;
        return true;
    }
    return false;
}

static bool find_removals(Grid *grid, int colors[], SimpleColoringStep *s) {
    int seen_color[81][2];
    for (int i = 0; i < 81; i++) {
        seen_color[i][0] = -1;
        seen_color[i][1] = -1;
    }

    for (int cell = 0; cell < 81; cell++) {
        if (colors[cell] == 0) continue;
        for (int peer_i = 0; peer_i < NUM_PEERS; peer_i++) {
            int peer = peers[cell][peer_i];
            seen_color[peer][colors[cell] - 1] = cell;
        }
    }

    if (check_twice_in_unit(colors, seen_color, s)) return true;
    if (check_emptied_unit(grid, seen_color, s)) return true;
    if (check_both_seen(grid, colors, seen_color, s)) return true;

    return false;
}
