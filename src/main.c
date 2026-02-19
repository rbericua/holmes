#include <stdio.h>

#include "grid.h"
#include "history.h"
#include "solver.h"
#include "step.h"
#include "ui.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: holmes <sudoku>\n");
        return 1;
    }

    Ui ui;

    Grid *grid = grid_create(argv[1]);
    History hist = {0};
    ui_init(&ui);

    ui_print_grid(&ui, grid, NULL);

    switch (ui_wait_for_input()) {
    case ACTION_QUIT: goto cleanup;
    default: break;
    }

    SolveStatus status;
    while (1) {
        Step step;
        status = solver_next_step(grid, &step);

        if (status != SOLVE_ONGOING) break;

        ui_print_grid(&ui, grid, &step);
        ui_print_step(&ui, &step);
        history_add(&hist, step);

    input:
        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_PREV:
            if (!history_undo(&hist, grid)) {
                ui_print_message(&ui, "Already at initial state\n");
            } else {
                ui_print_grid(&ui, grid, history_curr(&hist));
                ui_print_step(&ui, history_curr(&hist));
            }
            goto input;
        case ACTION_NEXT:
            if (!history_redo(&hist, grid)) {
                break;
            }
            ui_print_grid(&ui, grid, history_curr(&hist));
            ui_print_step(&ui, history_curr(&hist));
            goto input;
        case ACTION_SCROLL_UP: ui_scroll(&ui, -1); goto input;
        case ACTION_SCROLL_DOWN: ui_scroll(&ui, 1); goto input;
        }

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid, NULL);

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

    ui_wait_for_input();

cleanup:
    ui_deinit(&ui);
    history_free(&hist);
    grid_destroy(grid);

    return 0;
}
