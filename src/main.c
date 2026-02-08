#include <stdio.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "grid.h"
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

    ui_deinit(&ui);
    grid_destroy(grid);

    return 0;
}
