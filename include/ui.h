#ifndef UI_H
#define UI_H

#include <stdbool.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "grid.h"

typedef struct {
    WINDOW *grid_win;
    WINDOW *info_win;
} Ui;

void ui_init(Ui *ui);
void ui_deinit(Ui *ui);
void ui_print_message(Ui *ui, bool clear, char *format, ...);
void ui_print_grid(Ui *ui, Grid *grid);

#endif
