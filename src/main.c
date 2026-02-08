#include <stdio.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

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

    ui_print_grid(&ui, grid);
    getch();

    SolveStatus status;
    while (1) {
        Step step;
        status = solver_next_step(grid, &step);

        if (status != SOLVE_ONGOING) break;

        ui_print_grid(&ui, grid);
        ui_print_step(&ui, &step);
        getch();

        solver_apply_step(grid, &step);
    }

    ui_print_grid(&ui, grid);

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

    getch();

    ui_deinit(&ui);
    grid_destroy(grid);

    return 0;
}
