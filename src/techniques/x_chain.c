#include "techniques/x_chain.h"

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

typedef struct {
    int cell;
    LinkType type;
} CellLink;

typedef struct {
    CellLink links[NUM_PEERS];
    int len;
} CellLinks;

static int find_cell_link(CellLinks *cell_links, int cell);
static bool find_links(Grid *grid, int value, CellLinks out[81]);
static bool is_in_chain(int cell, XChain *chain);
static bool is_better_chain(int a_len, int a_num_removals, int b_len,
                            int b_num_removals);
static int find_strong_link_removals(Grid *grid, int value, int cell1,
                                     int cell2, int out[]);
static int find_ring_removals(Grid *grid, int value, XChain *chain, int out[]);
static bool try_update_best_chain(Grid *grid, int value, XChain *chain,
                                  XChainStep *s);
static bool find_chain(Grid *grid, int value, CellLinks grid_links[81],
                       int curr_cell, LinkType incoming_link,
                       XChain *curr_chain, XChainStep *s);
static bool find_best_chain(Grid *grid, int value, CellLinks grid_links[81],
                            XChainStep *s);

bool x_chain(Grid *grid, Step *step) {
    step->type = TECH_X_CHAIN;
    XChainStep *s = &step->as.x_chain;
    s->chain.len = 0;
    s->num_removals = 0;

    bool found = false;
    for (int value = 1; value <= 9; value++) {
        CellLinks grid_links[81];
        for (int i = 0; i < 81; i++) {
            grid_links[i].len = 0;
        }
        if (!find_links(grid, value, grid_links)) continue;

        if (find_best_chain(grid, value, grid_links, s)) {
            s->value = value;
            found = true;
        }
    }

    return found;
}

void x_chain_apply(Grid *grid, Step *step) {
    XChainStep *s = &step->as.x_chain;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->value);
    }
}

void x_chain_revert(Grid *grid, Step *step) {
    XChainStep *s = &step->as.x_chain;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->value);
    }
}

void x_chain_explain(DynStr *buf, Step *step) {
    XChainStep *s = &step->as.x_chain;

    char *chain_str = explain_x_chain(&s->chain, s->value);

    ds_append(buf, "[X-Chain%s] %s: \n", s->chain.is_ring ? " (Ring)" : "",
              chain_str);

    free(chain_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->value);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void x_chain_colorise(ColorPair colors[81][9], Step *step) {
    XChainStep *s = &step->as.x_chain;

    colors[s->chain.cells[0]][s->value - 1] = CP_TRIGGER;
    colors[s->chain.cells[s->chain.len - 1]][s->value - 1] = CP_TRIGGER;
    for (int i = 1; i < s->chain.len - 1; i++) {
        colors[s->chain.cells[i]][s->value - 1] = s->chain.is_ring
                                                      ? CP_TRIGGER
                                                      : CP_SPECIAL1;
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->value - 1] = CP_REMOVAL;
    }
}

void x_chain_pipes(Pipes *pipes, Step *step) {
    XChainStep *s = &step->as.x_chain;

    LinkType link = LINK_STRONG;
    for (int i = 0; i < s->chain.len - 1; i++) {
        da_append(pipes, pipe_create(s->chain.cells[i], s->value,
                                     s->chain.cells[i + 1], s->value, link));
        link = other_link(link);
    }
    if (s->chain.is_ring) {
        da_append(pipes, pipe_create(s->chain.cells[s->chain.len - 1], s->value,
                                     s->chain.cells[0], s->value, LINK_WEAK));
    }
}

static int find_cell_link(CellLinks *cell_links, int cell) {
    for (int i = 0; i < cell_links->len; i++) {
        if (cell_links->links[i].cell == cell) return i;
    }
    return -1;
}

static bool find_links(Grid *grid, int value, CellLinks out[81]) {
    bool found = false;

    for (int unit_i = 0; unit_i < 27; unit_i++) {
        int *unit = ALL_UNITS[unit_i];

        int cells[9];
        int num_cells = grid_region_with_cand(grid, unit, 9, value, cells);
        if (num_cells <= 1) continue;

        LinkType new_link_type = num_cells == 2 ? LINK_DUAL : LINK_WEAK;

        for (int cell_i = 0; cell_i < num_cells; cell_i++) {
            int *num_links = &out[cells[cell_i]].len;
            for (int cell_j = 0; cell_j < num_cells; cell_j++) {
                if (cell_i == cell_j) continue;

                int link_idx = find_cell_link(&out[cells[cell_i]],
                                              cells[cell_j]);
                if (link_idx == -1) {
                    out[cells[cell_i]].links[(*num_links)++] = (CellLink){
                        .cell = cells[cell_j],
                        .type = new_link_type,
                    };
                } else {
                    // If Pointing Set has run before, it's not necessary to
                    // update the link type
                    out[cells[cell_i]].links[link_idx].type |= new_link_type;
                }
            }
        }

        found = true;
    }

    return found;
}

static bool is_in_chain(int cell, XChain *chain) {
    for (int i = 0; i < chain->len; i++) {
        if (chain->cells[i] == cell) return true;
    }
    return false;
}

static bool is_better_chain(int a_len, int a_num_removals, int b_len,
                            int b_num_removals) {
    if (a_num_removals != b_num_removals) {
        return a_num_removals > b_num_removals;
    }
    return a_len < b_len;
}

static int find_strong_link_removals(Grid *grid, int value, int cell1,
                                     int cell2, int out[]) {
    int common[MAX_COMMON_PEERS];
    int num_common = cells_common_peers((int[]){cell1, cell2}, 2, common);
    return grid_region_with_cand(grid, common, num_common, value, out);
}

// TODO: Verify that a cell cannot be eliminated by two different strong links
static int find_ring_removals(Grid *grid, int value, XChain *chain, int out[]) {
    int num_removals = 0;
    for (int i = 1; i < chain->len; i += 2) {
        int j = i == chain->len - 1 ? 0 : i + 1;
        num_removals += find_strong_link_removals(
            grid, value, chain->cells[i], chain->cells[j], &out[num_removals]);
    }
    return num_removals;
}

static bool try_update_best_chain(Grid *grid, int value, XChain *chain,
                                  XChainStep *s) {
    int start = chain->cells[0];
    int end = chain->cells[chain->len - 1];
    if (end < start) return false; // Duplicate chain

    int removal_cells[MAX_COMMON_PEERS];
    int num_removals;

    bool is_ring = cells_are_peers(start, end);
    if (is_ring) {
        num_removals = find_ring_removals(grid, value, chain, removal_cells);
    } else {
        num_removals = find_strong_link_removals(grid, value, start, end,
                                                 removal_cells);
    }
    if (num_removals == 0) return false;

    if (is_better_chain(chain->len, num_removals, s->chain.len,
                        s->num_removals)) {
        memcpy(s->chain.cells, chain->cells, chain->len * sizeof(int));
        s->chain.len = chain->len;
        s->chain.is_ring = is_ring;
        memcpy(s->removal_cells, removal_cells, num_removals * sizeof(int));
        s->num_removals = num_removals;
        return true;
    }

    return false;
}

// incoming_link is either LINK_WEAK or LINK_STRONG
static bool find_chain(Grid *grid, int value, CellLinks grid_links[81],
                       int curr_cell, LinkType incoming_link,
                       XChain *curr_chain, XChainStep *s) {
    bool found = false;

    curr_chain->cells[curr_chain->len++] = curr_cell;

    if (incoming_link == LINK_STRONG && curr_chain->len >= 3) {
        if (try_update_best_chain(grid, value, curr_chain, s)) {
            found = true;
        }
    }

    if (curr_chain->len < MAX_X_CHAIN_LEN) {
        for (int i = 0; i < grid_links[curr_cell].len; i++) {
            int next_cell = grid_links[curr_cell].links[i].cell;
            LinkType next_link = grid_links[curr_cell].links[i].type;

            if (!links_are_alternating(incoming_link, next_link)
                || is_in_chain(next_cell, curr_chain))
                continue;

            if (find_chain(grid, value, grid_links, next_cell,
                           other_link(incoming_link), curr_chain, s)) {
                found = true;
            }
        }
    }

    curr_chain->len--;

    return found;
}

static bool find_best_chain(Grid *grid, int value, CellLinks grid_links[81],
                            XChainStep *s) {
    bool found = false;
    XChain chain = {.len = 0};
    for (int i = 0; i < 81; i++) {
        if (grid_links[i].len == 0) continue;
        chain.cells[0] = i;
        if (find_chain(grid, value, grid_links, i, LINK_WEAK, &chain, s)) {
            found = true;
        }
    }
    return found;
}
