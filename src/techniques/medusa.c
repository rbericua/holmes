#include "techniques/medusa.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
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

static char *explain_both_seen_rule(MedusaBothSeenRule rule);
static void explain_color_removals(DynStr *buf, MedusaStep *s);
static bool find_links(Grid *grid, int links[][9][3]);
static bool find_first_linked(int links[][9][3], int *cell, int *value);
static void paint_and_extract_links(Grid *grid, int links[][9][3], int cell,
                                    int value, int curr_color, int colors[][9],
                                    Link out_links[], int *num_links);
static int extract_chain(int colors[][9], MedusaNode chain[]);
static void mark_cell(int cell, int value, int color, int seen_color[][9][2]);
static void mark_peers(int cell, int value, int color, int seen_color[][9][2]);
static MedusaBothSeenRule get_both_seen_rule(int cell, int seen_color[]);
static int cands_emptied_by_color(CandSet cands, int seen_color[][2]);
static int cells_emptied_by_color(int cells[], int num_cells, int value,
                                  int seen_color[][9][2]);
static bool is_cell_on_chain(int cell, int colors[][9]);
static bool check_twice_in(int colors[][9], int seen_color[][9][2],
                           MedusaStep *s);
static bool check_emptied_cell(Grid *grid, int colors[][9],
                               int seen_color[][9][2], MedusaStep *s);
static bool check_emptied_unit(Grid *grid, int seen_color[][9][2],
                               MedusaStep *s);
static bool check_both_seen(Grid *grid, int colors[][9], int seen_color[][9][2],
                            MedusaStep *s);
static bool find_removals(Grid *grid, int colors[][9], MedusaStep *s);

bool medusa(Grid *grid, Step *step) {
    step->type = TECH_MEDUSA;
    MedusaStep *s = &step->as.medusa;

    int links[81][9][3];
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 9; j++) {
            for (int k = 0; k < 3; k++) {
                links[i][j][k] = -1;
            }
        }
    }
    if (!find_links(grid, links)) return false;

    int first_cell, first_value;
    while (find_first_linked(links, &first_cell, &first_value)) {
        int colors[81][9] = {0};
        s->num_links = 0;
        paint_and_extract_links(grid, links, first_cell, first_value, 1, colors,
                                s->links, &s->num_links);
        s->chain_len = extract_chain(colors, s->chain);

        if (find_removals(grid, colors, s)) return true;
    }

    return false;
}

void medusa_apply(Grid *grid, Step *step) {
    MedusaStep *s = &step->as.medusa;

    if (s->rule == MED_BOTH_SEEN) {
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            MedusaRemoval removal = s->both_seen.removals[i];
            grid_cell_remove_cand(grid, removal.cell, removal.cand);
        }
    } else {
        for (int i = 0; i < s->chain_len; i++) {
            if (s->chain[i].color == s->removal_color) {
                grid_cell_remove_cand(grid, s->chain[i].cell, s->chain[i].cand);
            }
        }
    }
}

void medusa_revert(Grid *grid, Step *step) {
    MedusaStep *s = &step->as.medusa;

    if (s->rule == MED_BOTH_SEEN) {
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            MedusaRemoval removal = s->both_seen.removals[i];
            grid_cell_add_cand(grid, removal.cell, removal.cand);
        }
    } else {
        for (int i = 0; i < s->chain_len; i++) {
            if (s->chain[i].color == s->removal_color) {
                grid_cell_add_cand(grid, s->chain[i].cell, s->chain[i].cand);
            }
        }
    }
}

void medusa_explain(DynStr *buf, Step *step) {
    MedusaStep *s = &step->as.medusa;

    switch (s->rule) {
    case MED_TWICE_IN_CELL: {
        char *cell_str = explain_cell(s->twice_in_cell.cell);

        ds_append(buf,
                  "[3D Medusa (Twice in Cell)] %s has multiple candidates of "
                  "the same color\n",
                  cell_str);

        free(cell_str);

        explain_color_removals(buf, s);
    } break;
    case MED_TWICE_IN_UNIT: {
        char *cells_str = explain_cells(s->twice_in_unit.cells, 2);

        ds_append(buf,
                  "[3D Medusa (Twice in Unit)] %s see each other and have %ds "
                  "of the same color\n",
                  cells_str, s->twice_in_unit.value);

        free(cells_str);

        explain_color_removals(buf, s);
    } break;
    case MED_BOTH_SEEN:
        ds_append(buf, "[3D Medusa (Both Seen)]\n");
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            MedusaRemoval removal = s->both_seen.removals[i];

            char *removal_msg = explain_value_removal(removal.cell,
                                                      removal.cand);
            char *rule_str = explain_both_seen_rule(removal.rule);

            ds_append(buf, "%s (%s)\n", removal_msg, rule_str);

            free(removal_msg);
            free(rule_str);
        }
        break;
    case MED_EMPTIED_CELL: {
        char *cell_str = explain_cell(s->emptied_cell.cell);

        ds_append(buf,
                  "[3D Medusa (Emptied Cell)] Every candidate in %s sees the "
                  "same color\n",
                  cell_str);

        free(cell_str);

        explain_color_removals(buf, s);
    } break;
    case MED_EMPTIED_UNIT: {
        char *unit_str = explain_unit_name(s->emptied_unit.unit_type);

        ds_append(buf,
                  "[3D Medusa (Emptied Unit)] Every %d in %s %d sees the same "
                  "color\n",
                  s->emptied_unit.value, unit_str,
                  s->emptied_unit.unit_idx + 1);

        free(unit_str);

        explain_color_removals(buf, s);
    } break;
    }
}

void medusa_colorise(ColorPair colors[81][9], Step *step) {
    MedusaStep *s = &step->as.medusa;

    for (int i = 0; i < s->chain_len; i++) {
        int color;
        if (s->chain[i].color == s->removal_color) {
            color = CP_REMOVAL;
        } else {
            color = s->chain[i].color == 1 ? CP_SPECIAL1 : CP_SPECIAL2;
        }
        colors[s->chain[i].cell][s->chain[i].cand - 1] = color;
    }

    switch (s->rule) {
    case MED_BOTH_SEEN:
        for (int i = 0; i < s->both_seen.num_removals; i++) {
            MedusaRemoval removal = s->both_seen.removals[i];
            colors[removal.cell][removal.cand - 1] = CP_REMOVAL;
        }
        break;
    case MED_EMPTIED_CELL:
        for (int i = 0; i < 9; i++) {
            colors[s->emptied_cell.cell][i] = CP_TRIGGER;
        }
        break;
    case MED_EMPTIED_UNIT:
        for (int i = 0; i < s->emptied_unit.num_emptied_cells; i++) {
            int cell = s->emptied_unit.emptied_cells[i];
            colors[cell][s->emptied_unit.value - 1] = CP_TRIGGER;
        }
        break;
    default: break;
    }
}

void medusa_pipes(Pipes *pipes, Step *step) {
    MedusaStep *s = &step->as.medusa;

    for (int i = 0; i < s->num_links; i++) {
        Link link = s->links[i];
        da_append(pipes,
                  pipe_create(link.cell1, link.cand1, link.cell2, link.cand2));
    }
}

static char *explain_both_seen_rule(MedusaBothSeenRule rule) {
    switch (rule) {
    case MED_BOTH_SEEN_CELL: return strdup("Cell");
    case MED_BOTH_SEEN_UNITS: return strdup("Units");
    case MED_BOTH_SEEN_CELL_UNIT: return strdup("Cell and Unit");
    default: return strdup("");
    }
}

static void explain_color_removals(DynStr *buf, MedusaStep *s) {
    for (int i = 0; i < s->chain_len; i++) {
        if (s->chain[i].color == s->removal_color) {
            char *removal_msg = explain_value_removal(s->chain[i].cell,
                                                      s->chain[i].cand);
            ds_append(buf, "%s\n", removal_msg);
            free(removal_msg);
        }
    }
}

static bool find_links(Grid *grid, int links[][9][3]) {
    bool found = false;
    for (int value = 1; value <= 9; value++) {
        for (int unit_type = 0; unit_type < 3; unit_type++) {
            for (int unit_i = 0; unit_i < 9; unit_i++) {
                int *unit = units[unit_type][unit_i];
                int pair[9];
                if (grid_region_with_cand(grid, unit, 9, value, pair) != 2)
                    continue;
                links[pair[0]][value - 1][unit_type] = pair[1];
                links[pair[1]][value - 1][unit_type] = pair[0];
                found = true;
            }
        }
    }
    return found;
}

static bool find_first_linked(int links[][9][3], int *cell, int *value) {
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 9; j++) {
            for (int k = 0; k < 3; k++) {
                if (links[i][j][k] != -1) {
                    *cell = i;
                    *value = j + 1;
                    return true;
                }
            }
        }
    }
    return false;
}

static void paint_and_extract_links(Grid *grid, int links[][9][3], int cell,
                                    int value, int curr_color, int colors[][9],
                                    Link out_links[], int *num_links) {
    colors[cell][value - 1] = curr_color;

    if (grid_cell_num_cands(grid, cell) == 2) {
        int other_cand = cand_set_first(
            cand_set_remove(grid_cell_cands(grid, cell), value));

        if (colors[cell][other_cand - 1] == 0) {
            out_links[(*num_links)++] = (Link){
                .cell1 = cell,
                .cand1 = value,
                .cell2 = cell,
                .cand2 = other_cand,
            };

            paint_and_extract_links(grid, links, cell, other_cand,
                                    other_color(curr_color), colors, out_links,
                                    num_links);
        }
    }

    for (int unit_type = 0; unit_type < 3; unit_type++) {
        int other_cell = links[cell][value - 1][unit_type];
        if (other_cell == -1) continue;

        links[cell][value - 1][unit_type] = -1;
        links[other_cell][value - 1][unit_type] = -1;

        if (colors[other_cell][value - 1] == 0) {
            out_links[(*num_links)++] = (Link){
                .cell1 = cell,
                .cand1 = value,
                .cell2 = other_cell,
                .cand2 = value,
            };

            paint_and_extract_links(grid, links, other_cell, value,
                                    other_color(curr_color), colors, out_links,
                                    num_links);
        }
    }
}

static int extract_chain(int colors[][9], MedusaNode chain[]) {
    int count = 0;
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 9; j++) {
            if (colors[i][j] != 0) {
                chain[count++] = (MedusaNode){
                    .cell = i,
                    .cand = j + 1,
                    .color = colors[i][j],
                };
            }
        }
    }
    return count;
}

static void mark_cell(int cell, int value, int color, int seen_color[][9][2]) {
    for (int i = 0; i < 9; i++) {
        if (i != value - 1) {
            seen_color[cell][i][color - 1] = cell;
        }
    }
}

static void mark_peers(int cell, int value, int color, int seen_color[][9][2]) {
    for (int peer_i = 0; peer_i < NUM_PEERS; peer_i++) {
        int peer = peers[cell][peer_i];
        seen_color[peer][value - 1][color - 1] = cell;
    }
}

static MedusaBothSeenRule get_both_seen_rule(int cell, int seen_color[]) {
    if (seen_color[0] == cell && seen_color[1] == cell)
        return MED_BOTH_SEEN_CELL;
    if (seen_color[0] != cell && seen_color[1] != cell)
        return MED_BOTH_SEEN_UNITS;
    return MED_BOTH_SEEN_CELL_UNIT;
}

static int cands_emptied_by_color(CandSet cands, int seen_color[][2]) {
    CandSet emptied_cands[] = {CAND_SET_EMPTY, CAND_SET_EMPTY};
    for (int value = 1; value <= 9; value++) {
        for (int color = 0; color < 2; color++) {
            if (seen_color[value - 1][color] != -1) {
                emptied_cands[color] = cand_set_add(emptied_cands[color],
                                                    value);
            }
        }
    }
    for (int color = 0; color < 2; color++) {
        if (cand_set_is_sub(cands, emptied_cands[color])) return color + 1;
    }
    return 0;
}

static int cells_emptied_by_color(int cells[], int num_cells, int value,
                                  int seen_color[][9][2]) {
    bool emptied_by[] = {true, true};
    for (int cell_i = 0; cell_i < num_cells; cell_i++) {
        int cell = cells[cell_i];
        for (int color = 0; color < 2; color++) {
            if (seen_color[cell][value - 1][color] == -1) {
                emptied_by[color] = false;
            }
        }
    }
    for (int color = 0; color < 2; color++) {
        if (emptied_by[color]) return color + 1;
    }
    return 0;
}

static bool is_cell_on_chain(int cell, int colors[][9]) {
    for (int i = 0; i < 9; i++) {
        if (colors[cell][i] != 0) return true;
    }
    return false;
}

static bool check_twice_in(int colors[][9], int seen_color[][9][2],
                           MedusaStep *s) {
    for (int cell = 0; cell < 81; cell++) {
        for (int value = 1; value <= 9; value++) {
            if (seen_color[cell][value - 1][0] == -1
                || seen_color[cell][value - 1][1] == -1)
                continue;

            int color = colors[cell][value - 1];
            if (color != 0) {
                int other_cell = seen_color[cell][value - 1][color - 1];
                if (other_cell == cell) {
                    s->rule = MED_TWICE_IN_CELL;
                    s->twice_in_cell.cell = cell;
                    s->removal_color = color;
                } else {
                    s->rule = MED_TWICE_IN_UNIT;
                    s->twice_in_unit.value = value;
                    s->twice_in_unit.cells[0] = cell;
                    s->twice_in_unit.cells[1] = other_cell;
                    s->removal_color = color;
                }
                return true;
            }
        }
    }
    return false;
}

static bool check_emptied_cell(Grid *grid, int colors[][9],
                               int seen_color[][9][2], MedusaStep *s) {
    for (int cell = 0; cell < 81; cell++) {
        if (!grid_cell_is_empty(grid, cell) || is_cell_on_chain(cell, colors))
            continue;

        int emptied_by = cands_emptied_by_color(grid_cell_cands(grid, cell),
                                                seen_color[cell]);
        if (emptied_by != 0) {
            s->rule = MED_EMPTIED_CELL;
            s->emptied_cell.cell = cell;
            s->removal_color = emptied_by;
            return true;
        }
    }
    return false;
}

static bool check_emptied_unit(Grid *grid, int seen_color[][9][2],
                               MedusaStep *s) {
    for (int unit_type = 0; unit_type < 3; unit_type++) {
        for (int unit_i = 0; unit_i < 9; unit_i++) {
            for (int value = 1; value <= 9; value++) {
                int cells_to_empty[9];
                int num_cells_to_empty = grid_region_with_cand(
                    grid, units[unit_type][unit_i], 9, value, cells_to_empty);
                if (num_cells_to_empty == 0) continue;

                int emptied_by = cells_emptied_by_color(
                    cells_to_empty, num_cells_to_empty, value, seen_color);
                if (emptied_by != 0) {
                    s->rule = MED_EMPTIED_UNIT;
                    s->emptied_unit.value = value;
                    s->emptied_unit.unit_idx = unit_i;
                    s->emptied_unit.unit_type = unit_type;
                    memcpy(s->emptied_unit.emptied_cells, cells_to_empty,
                           num_cells_to_empty * sizeof(int));
                    s->emptied_unit.num_emptied_cells = num_cells_to_empty;
                    s->removal_color = emptied_by;
                    return true;
                }
            }
        }
    }
    return false;
}

static bool check_both_seen(Grid *grid, int colors[][9], int seen_color[][9][2],
                            MedusaStep *s) {
    s->both_seen.num_removals = 0;
    for (int cell = 0; cell < 81; cell++) {
        for (int value = 1; value <= 9; value++) {
            if (seen_color[cell][value - 1][0] == -1
                || seen_color[cell][value - 1][1] == -1)
                continue;

            int color = colors[cell][value - 1];
            if (color == 0 && grid_cell_has_cand(grid, cell, value)) {
                MedusaRemoval removal = {
                    .cell = cell,
                    .cand = value,
                    .rule = get_both_seen_rule(cell,
                                               seen_color[cell][value - 1]),
                };
                s->both_seen.removals[s->both_seen.num_removals++] = removal;
            }
        }
    }
    if (s->both_seen.num_removals > 0) {
        s->rule = MED_BOTH_SEEN;
        s->removal_color = -1;
        return true;
    }
    return false;
}

static bool find_removals(Grid *grid, int colors[][9], MedusaStep *s) {
    int seen_color[81][9][2];
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 9; j++) {
            seen_color[i][j][0] = -1;
            seen_color[i][j][1] = -1;
        }
    }

    for (int cell = 0; cell < 81; cell++) {
        for (int value = 1; value <= 9; value++) {
            int color = colors[cell][value - 1];
            if (color == 0) continue;

            mark_cell(cell, value, color, seen_color);
            mark_peers(cell, value, color, seen_color);
        }
    }

    if (check_twice_in(colors, seen_color, s)) return true;
    if (check_emptied_cell(grid, colors, seen_color, s)) return true;
    if (check_emptied_unit(grid, seen_color, s)) return true;
    if (check_both_seen(grid, colors, seen_color, s)) return true;

    return false;
}
