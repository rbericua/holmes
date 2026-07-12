#include "techniques/y_wing.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cand_set.h"
#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "techniques/explain.h"
#include "ui/colors.h"
#include "util/combinations.h"
#include "util/dynstr.h"

static int find_hinge(Grid *grid, int common_peers[], int num_common_peers,
                      CandSet hinge_cands);

bool y_wing(Grid *grid, Step *step) {
    step->type = TECH_Y_WING;
    YWingStep *s = &step->as.y_wing;

    int possible_wings[81];
    int num_possible_wings = grid_region_with_n_cands_exact(
        grid, (int *)units[UNIT_ROW], 81, 2, possible_wings);

    int num_wing_combs;
    int **wing_combs = generate_combinations(possible_wings, num_possible_wings,
                                             2, sizeof(int), &num_wing_combs);

    for (int wing_comb_i = 0; wing_comb_i < num_wing_combs; wing_comb_i++) {
        int *wings = wing_combs[wing_comb_i];
        CandSet wing_cands[] = {grid_cell_cands(grid, wings[0]),
                                grid_cell_cands(grid, wings[1])};

        if (cand_set_len(wing_cands[0] | wing_cands[1]) != 3) continue;

        CandSet hinge_cands = wing_cands[0] ^ wing_cands[1];

        int common_peers[MAX_COMMON_PEERS];
        int num_common_peers = cells_common_peers(wings, 2, common_peers);

        int hinge = find_hinge(grid, common_peers, num_common_peers,
                               hinge_cands);
        if (hinge == -1) continue;

        s->removal_cand = cand_set_first(wing_cands[0] & wing_cands[1]);
        s->num_removals = grid_region_with_cand(
            grid, common_peers, num_common_peers, s->removal_cand,
            s->removal_cells);

        if (s->num_removals == 0) continue;

        s->hinge = hinge;
        s->hinge_cands = hinge_cands;
        memcpy(s->wings, wings, 2 * sizeof(int));
        memcpy(s->wing_cands, wing_cands, 2 * sizeof(CandSet));

        free_combinations(wing_combs);

        return true;
    }

    free_combinations(wing_combs);

    return false;
}

void y_wing_apply(Grid *grid, Step *step) {
    YWingStep *s = &step->as.y_wing;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_remove_cand(grid, s->removal_cells[i], s->removal_cand);
    }
}

void y_wing_revert(Grid *grid, Step *step) {
    YWingStep *s = &step->as.y_wing;

    for (int i = 0; i < s->num_removals; i++) {
        grid_cell_add_cand(grid, s->removal_cells[i], s->removal_cand);
    }
}

void y_wing_explain(DynStr *buf, Step *step) {
    YWingStep *s = &step->as.y_wing;

    char *hinge_str = explain_cell(s->hinge);
    char *hinge_cands_str = explain_cand_set(s->hinge_cands);
    char *wing_strs[] = {explain_cell(s->wings[0]), explain_cell(s->wings[1])};
    char *wing_cands_strs[] = {explain_cand_set(s->wing_cands[0]),
                               explain_cand_set(s->wing_cands[1])};

    ds_append(buf, "[Y-Wing] Hinge on %s %s, wings on %s %s, %s %s:\n",
              hinge_str, hinge_cands_str, wing_strs[0], wing_cands_strs[0],
              wing_strs[1], wing_cands_strs[1]);

    free(hinge_str);
    free(hinge_cands_str);
    free(wing_strs[0]);
    free(wing_strs[1]);
    free(wing_cands_strs[0]);
    free(wing_cands_strs[1]);

    for (int i = 0; i < s->num_removals; i++) {
        char *removal_msg = explain_value_removal(s->removal_cells[i],
                                                  s->removal_cand);
        ds_append(buf, "%s\n", removal_msg);
        free(removal_msg);
    }
}

void y_wing_colorise(ColorPair colors[81][9], Step *step) {
    YWingStep *s = &step->as.y_wing;

    for (int i = 0; i < 9; i++) {
        colors[s->hinge][i] = CP_SPECIAL1;
        colors[s->wings[0]][i] = CP_SPECIAL1;
        colors[s->wings[1]][i] = CP_SPECIAL1;
    }
    colors[s->wings[0]][s->removal_cand - 1] = CP_TRIGGER;
    colors[s->wings[1]][s->removal_cand - 1] = CP_TRIGGER;
    for (int i = 0; i < s->num_removals; i++) {
        colors[s->removal_cells[i]][s->removal_cand - 1] = CP_REMOVAL;
    }
}

static int find_hinge(Grid *grid, int common_peers[], int num_common_peers,
                      CandSet hinge_cands) {
    for (int i = 0; i < num_common_peers; i++) {
        int cell = common_peers[i];
        if (grid_cell_cands(grid, cell) == hinge_cands) return cell;
    }
    return -1;
}
