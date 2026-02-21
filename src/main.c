#include <stdio.h>

#include "grid.h"
#include "history.h"
#include "solver.h"
#include "step.h"
#include "ui.h"
#include "techniques/backtrack.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: holmes <sudoku>\n");
        return 1;
    }

    Ui ui;

    Grid *grid = grid_create(argv[1]);
    History hist = {0};
    ui_init(&ui);

    int num_solutions = backtrack(grid);

    ui_print_grid(&ui, grid, NULL);

    if (num_solutions != 1) {
        if (num_solutions == 0) {
            ui_print_message(&ui, true, true,
                             "Invalid Sudoku. Found no solutions\n");
        } else if (num_solutions > 1) {
            ui_print_message(&ui, true, true,
                             "Invalid Sudoku. Found multiple solutions\n");
        }

        while (true) {
            switch (ui_wait_for_input()) {
            case ACTION_QUIT:
            case ACTION_NEXT: goto cleanup;
            default: break;
            }
        }
    }

    bool waiting = true;
    while (waiting) {
        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_NEXT: waiting = false;
        default: break;
        }
    }

    SolveStatus status;
    while (1) {
        Step step;
        status = solver_next_step(grid, &step);

        if (status != SOLVE_ONGOING) break;

        ui_print_grid(&ui, grid, &step);
        ui_print_step(&ui, &step);
        history_add(&hist, step);

        waiting = true;
        while (waiting) {
            switch (ui_wait_for_input()) {
            case ACTION_QUIT: goto cleanup;
            case ACTION_PREV:
                if (!history_undo(&hist, grid)) {
                    ui_print_message(&ui, true, true,
                                     "Already at initial state\n");
                } else {
                    ui_print_grid(&ui, grid, history_curr(&hist));
                    ui_print_step(&ui, history_curr(&hist));
                }
                break;
            case ACTION_NEXT:
                if (!history_redo(&hist, grid)) {
                    waiting = false;
                    break;
                }
                ui_print_grid(&ui, grid, history_curr(&hist));
                ui_print_step(&ui, history_curr(&hist));
                break;
            case ACTION_SCROLL_UP: ui_scroll(&ui, -1); break;
            case ACTION_SCROLL_DOWN: ui_scroll(&ui, 1); break;
            }
        }

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid, NULL);

    switch (status) {
    case SOLVE_COMPLETE:
        ui_print_message(&ui, true, true, "Sudoku solved successfully\n");
        break;
    case SOLVE_STUCK:
        ui_print_message(&ui, true, true,
                         "Solver stuck. No further progress possible with "
                         "available techniques\n");
        break;
    default: break;
    }

    while (true) {
        switch (ui_wait_for_input()) {
        case ACTION_QUIT:
        case ACTION_NEXT: goto cleanup;
        default: break;
        }
    }

cleanup:
    ui_deinit(&ui);
    history_free(&hist);
    grid_destroy(grid);

    return 0;
}
