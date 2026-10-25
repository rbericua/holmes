#include "techniques/xy_chain.h"

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

#define CHAIN_START_CAND(c) ((c)->chain[0])
#define CHAIN_START_CELL(c) ((c)->chain[1])
#define CHAIN_END_CAND(c) ((c)->chain[(c)->arr_len - 1])
#define CHAIN_END_CELL(c) ((c)->chain[(c)->arr_len - 2])

typedef struct {
    // Could it be smaller?
    int links[NUM_PEERS];
    int len;
} CellLinks;

static bool find_links(Grid *grid, CellLinks out[81]);
static bool is_in_chain(int cell, XyChain *chain);
static bool is_better_chain(int a_len, int a_num_removals, int b_len,
                            int b_num_removals);
static int find_strong_link_removals(Grid *grid, int value, int cell1,
                                     int cell2, CellCand out[]);
static int find_ring_removals(Grid *grid, XyChain *chain, CellCand out[]);
static bool try_update_best_chain(Grid *grid, XyChain *chain, XyChainStep *s);
static bool find_chain(Grid *grid, CellLinks grid_links[81], int curr_cell,
                       int curr_cand, XyChain *curr_chain, XyChainStep *s);
static bool find_best_chain(Grid *grid, CellLinks grid_links[81],
                            XyChainStep *s);

bool xy_chain(Grid *grid, Step *step) {
    step->type = TECH_XY_CHAIN;
    XyChainStep *s = &step->as.xy_chain;
    s->chain.chain_len = 0;
    s->num_removals = 0;

    CellLinks grid_links[81];
    for (int i = 0; i < 81; i++) {
        grid_links[i].len = 0;
    }
    if (!find_links(grid, grid_links)) return false;

    return find_best_chain(grid, grid_links, s);
}

void xy_chain_apply(Grid *grid, Step *step) {
    XyChainStep *s = &step->as.xy_chain;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removals[i].cell, s->removals[i].cand);
    }
}

void xy_chain_revert(Grid *grid, Step *step) {
    XyChainStep *s = &step->as.xy_chain;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removals[i].cell, s->removals[i].cand);
    }
}

void xy_chain_explain(DynStr *buf, Step *step) {
    XyChainStep *s = &step->as.xy_chain;

    char *chain_str = explain_xy_chain(&s->chain);

    ds_append(buf, "[XY-Chain%s] %s:\n", s->chain.is_ring ? " (Ring)" : "",
              chain_str);

    free(chain_str);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removals[i].cell,
                                                  s->removals[i].cand);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void xy_chain_colorise(ColorPair colors[81][9], Step *step) {
    XyChainStep *s = &step->as.xy_chain;

    for (int i = 1; i < s->chain.arr_len - 1; i += 2) {
        int color = s->chain.is_ring ? CP_TRIGGER : CP_SPECIAL1;
        colors[s->chain.chain[i]][s->chain.chain[i - 1] - 1] = color;
        colors[s->chain.chain[i]][s->chain.chain[i + 1] - 1] = color;
    }
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removals[i].cell][s->removals[i].cand - 1] = CP_REMOVAL;
    }
    colors[CHAIN_START_CELL(&s->chain)][CHAIN_START_CAND(&s->chain) - 1] =
        CP_TRIGGER;
    colors[CHAIN_END_CELL(&s->chain)][CHAIN_END_CAND(&s->chain) - 1] =
        CP_TRIGGER;
}

void xy_chain_pipes(Pipes *pipes, Step *step) {
    XyChainStep *s = &step->as.xy_chain;

    for (int i = 1; i < s->chain.arr_len - 1; i += 2) {
        int j = i == s->chain.arr_len - 2 ? 1 : i + 2;
        da_append(pipes, pipe_create(s->chain.chain[i], s->chain.chain[i - 1],
                                     s->chain.chain[i], s->chain.chain[i + 1],
                                     LINK_STRONG));
        if (s->chain.is_ring || j != 1) {
            da_append(pipes,
                      pipe_create(s->chain.chain[i], s->chain.chain[i + 1],
                                  s->chain.chain[j], s->chain.chain[i + 1],
                                  LINK_WEAK));
        }
    }
}

static bool find_links(Grid *grid, CellLinks out[81]) {
    bool found = false;

    for (int unit_i = 0; unit_i < 27; unit_i++) {
        int *unit = ALL_UNITS[unit_i];

        int cells[9];
        int num_cells = grid_region_with_n_cands_exact(grid, unit, 9, 2, cells);
        if (num_cells <= 1) continue;

        for (int cell_i = 0; cell_i < num_cells - 1; cell_i++) {
            int *num_links_i = &out[cells[cell_i]].len;
            for (int cell_j = cell_i + 1; cell_j < num_cells; cell_j++) {
                int *num_links_j = &out[cells[cell_j]].len;

                CandSet common_cands = grid_cell_cands(grid, cells[cell_i])
                                       & grid_cell_cands(grid, cells[cell_j]);
                if (cand_set_is_empty(common_cands)) continue;

                out[cells[cell_i]].links[(*num_links_i)++] = cells[cell_j];
                out[cells[cell_j]].links[(*num_links_j)++] = cells[cell_i];

                found = true;
            }
        }
    }

    return found;
}

static bool is_in_chain(int cell, XyChain *chain) {
    for (int i = 1; i < chain->arr_len - 1; i += 2) {
        if (cell == chain->chain[i]) return true;
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
                                     int cell2, CellCand out[]) {
    int common[MAX_COMMON_PEERS];
    int num_common = cells_common_peers((int[]){cell1, cell2}, 2, common);
    int num_removals = grid_region_with_cand(grid, common, num_common, value,
                                             common);
    for (int i = 0; i < num_removals; i++) {
        out[i] = (CellCand){common[i], value};
    }
    return num_removals;
}

static int find_ring_removals(Grid *grid, XyChain *chain, CellCand out[]) {
    int num_removals = 0;
    for (int i = 1; i < chain->arr_len - 1; i += 2) {
        int j = i == chain->arr_len - 2 ? 1 : i + 2;
        num_removals += find_strong_link_removals(
            grid, chain->chain[i + 1], chain->chain[i], chain->chain[j],
            &out[num_removals]);
    }
    return num_removals;
}

static bool try_update_best_chain(Grid *grid, XyChain *chain, XyChainStep *s) {
    int start_cand = CHAIN_START_CAND(chain);
    int end_cand = CHAIN_END_CAND(chain);
    int start_cell = CHAIN_START_CELL(chain);
    int end_cell = CHAIN_END_CELL(chain);
    if (start_cell > end_cell || start_cand != end_cand) return false;

    CellCand removals[81];
    int num_removals;

    bool is_ring = cells_are_peers(start_cell, end_cell);
    if (is_ring) {
        num_removals = find_ring_removals(grid, chain, removals);
    } else {
        num_removals = find_strong_link_removals(grid, start_cand, start_cell,
                                                 end_cell, removals);
    }
    if (num_removals == 0) return false;

    if (is_better_chain(chain->chain_len, num_removals, s->chain.chain_len,
                        s->num_removals)) {
        s->chain = *chain;
        s->chain.is_ring = is_ring;
        memcpy(s->removals, removals, num_removals * sizeof(CellCand));
        s->num_removals = num_removals;
        return true;
    }

    return false;
}

static bool find_chain(Grid *grid, CellLinks grid_links[81], int curr_cell,
                       int curr_cand, XyChain *curr_chain, XyChainStep *s) {
    bool found = false;

    curr_chain->chain[curr_chain->arr_len++] = curr_cell;
    curr_chain->chain_len++;

    int next_cand = cand_set_first(
        cand_set_remove(grid_cell_cands(grid, curr_cell), curr_cand));
    curr_chain->chain[curr_chain->arr_len++] = next_cand;

    if (curr_chain->chain_len >= 2) {
        if (try_update_best_chain(grid, curr_chain, s)) {
            found = true;
        }
    }

    if (curr_chain->chain_len < MAX_XY_CHAIN_LEN) {
        for (int i = 0; i < grid_links[curr_cell].len; i++) {
            int next_cell = grid_links[curr_cell].links[i];
            if (!grid_cell_has_cand(grid, next_cell, next_cand)
                || is_in_chain(next_cell, curr_chain))
                continue;

            if (find_chain(grid, grid_links, next_cell, next_cand, curr_chain,
                           s)) {
                found = true;
            }
        }
    }

    curr_chain->arr_len -= 2;
    curr_chain->chain_len--;

    return found;
}

static bool find_best_chain(Grid *grid, CellLinks grid_links[81],
                            XyChainStep *s) {
    bool found = false;
    XyChain chain = {.arr_len = 1, .chain_len = 0};
    for (int i = 0; i < 81; i++) {
        if (grid_links[i].len == 0) continue;
        CandSet cands = grid_cell_cands(grid, i);
        while (!cand_set_is_empty(cands)) {
            int cand = cand_set_pop(&cands);
            chain.chain[0] = cand;
            if (find_chain(grid, grid_links, i, cand, &chain, s)) {
                found = true;
            }
        }
    }
    return found;
}
