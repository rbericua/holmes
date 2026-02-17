#include <stdio.h>

#include "grid.h"
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

    input:
        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_NEXT: break;
        case ACTION_SCROLL_UP: ui_scroll(&ui, -1); goto input;
        case ACTION_SCROLL_DOWN: ui_scroll(&ui, 1); goto input;
        }

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid, NULL);

    switch (status) {
    case SOLVE_COMPLETE:
        ui_print_message(&ui, "Sudoku solved successfully");
        break;
    case SOLVE_STUCK:
        ui_print_message(&ui, "Solver stuck. No further progress possible with "
                              "available techniques");
        break;
    default: break;
    }

    ui_wait_for_input();

cleanup:
    ui_deinit(&ui);
    grid_destroy(grid);

    return 0;
}
