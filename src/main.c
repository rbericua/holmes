#include <stdio.h>

#include "geometry.h"
#include "grid.h"
#include "solver.h"
#include "step.h"
#include "ui.h"
#include "util/log.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: holmes <sudoku>\n");
        return 1;
    }

    Ui ui;

    log_init();
    log_set_level(LOG_DEBUG);
    geometry_init();
    Grid *grid = grid_create(argv[1]);
    ui_init(&ui);

    ui_print_grid(&ui, grid, NULL);

    bool waiting = true;
    while (waiting) {
        switch (ui_wait_for_input()) {
        case ACTION_QUIT: goto cleanup;
        case ACTION_NEXT: waiting = false; break;
        default: break;
        }
    }

    SolveStatus status;
    while (true) {
        Step step;
        status = solver_next_step(grid, &step);
        if (status != SOLVE_ONGOING) break;

        ui_print_grid(&ui, grid, &step);
        ui_print_step(&ui, &step);

        waiting = true;
        while (waiting) {
            switch (ui_wait_for_input()) {
            case ACTION_QUIT: goto cleanup;
            case ACTION_NEXT: waiting = false; break;
            case ACTION_SCROLL_DOWN: ui_scroll(&ui, 1); break;
            case ACTION_SCROLL_UP: ui_scroll(&ui, -1); break;
            }
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
    grid_destroy(grid);
    log_deinit();

    return 0;
}
