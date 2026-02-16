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
    case ACTION_NEXT: break;
    }

    SolveStatus status;
    while (1) {
        Step step;
        status = solver_next_step(grid, &step);

        if (status != SOLVE_ONGOING) break;

        ui_print_grid(&ui, grid, &step);
        ui_print_step(&ui, &step);

        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_NEXT: break;
        }

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid, NULL);

    switch (status) {
    case SOLVE_COMPLETE:
        ui_print_message(&ui, true, "Sudoku solved successfully");
        break;
    case SOLVE_STUCK:
        ui_print_message(&ui, true,
                         "Solver stuck. No further progress possible with "
                         "available techniques");
        break;
    default: break;
    }

    switch (ui_wait_for_input()) {
    case ACTION_QUIT: goto cleanup;
    case ACTION_NEXT: break;
    }

cleanup:
    ui_deinit(&ui);
    grid_destroy(grid);

    return 0;
}
