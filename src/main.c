#include <stdio.h>

#include "geometry.h"
#include "grid.h"
#include "history.h"
#include "solver.h"
#include "step.h"
#include "techniques/backtrack.h"
#include "ui/ui.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: holmes <sudoku>\n");
        return 1;
    }

    log_init();
    log_set_level(LOG_DEBUG);

    Ui ui;

    geometry_init();
    Grid *grid = grid_create(argv[1]);
    History hist = {0};
    ui_init(&ui);

    bool bad_sudoku = false;

    int num_clues = 81 - grid_num_empty(grid);
    if (num_clues < 17) {
        ui_print_message(&ui, "Invalid Sudoku. Must have at least 17 clues\n");
        bad_sudoku = true;
    } else {
        int num_solutions = backtrack(grid);
        if (num_solutions != 1) {
            ui_print_message(&ui, "Invalid Sudoku. Found %s solutions\n",
                             num_solutions == 0 ? "no" : "multiple");
            bad_sudoku = true;
        }
    }

    ui_print_grid(&ui, grid, NULL, false);

    bool waiting = true;
    while (waiting) {
        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_NEXT: {
            if (bad_sudoku) {
                goto cleanup;
            } else {
                waiting = false;
            }
            break;
        }
        default: break;
        }
    }

    char grid_str[GRID_STR_LEN + 1];

    SolveStatus status;
    while (true) {
        Step step;
        status = solver_next_step(grid, &step);
        if (status != SOLVE_ONGOING) break;

        history_save(&hist, step);
        ui_print_grid(&ui, grid, &step, false);
        ui_print_step(&ui, &step);

        bool show_pipes = false;
        waiting = true;
        while (waiting) {
            switch (ui_wait_for_input()) {
            case ACTION_QUIT: goto cleanup;
            case ACTION_NEXT:
                if (history_redo(&hist, grid)) {
                    ui_print_grid(&ui, grid, history_curr(&hist), false);
                    ui_print_step(&ui, history_curr(&hist));
                } else {
                    waiting = false;
                }
                break;
            case ACTION_PREV:
                if (history_undo(&hist, grid)) {
                    ui_print_grid(&ui, grid, history_curr(&hist), false);
                    ui_print_step(&ui, history_curr(&hist));
                } else {
                    ui_print_message(&ui, "Already at initial state\n");
                }
                break;
            case ACTION_SCROLL_DOWN: ui_scroll(&ui, 1); break;
            case ACTION_SCROLL_UP: ui_scroll(&ui, -1); break;
            case ACTION_TOGGLE_PIPES:
                show_pipes = !show_pipes;
                ui_print_grid(&ui, grid, history_curr(&hist), show_pipes);
                break;
            case ACTION_EXPORT:
                grid_encode(grid, grid_str);
                ui_print_message(&ui, "Current grid state: %s\n", grid_str);
            }
        }

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid, NULL, false);

    switch (status) {
    case SOLVE_COMPLETE:
        ui_print_message(&ui, "Sudoku solved successfully\n");
        break;
    case SOLVE_STUCK:
        ui_print_message(&ui, "Solver stuck. No further progress possible with "
                              "available techniques\n");
        break;
    default: break;
    }

    waiting = true;
    while (waiting) {
        switch (ui_wait_for_input()) {
        case ACTION_QUIT:
        case ACTION_NEXT: waiting = false; break;
        default: break;
        }
    }

cleanup:
    ui_deinit(&ui);
    history_deinit(&hist);
    grid_destroy(grid);
    log_deinit();

    return 0;
}
