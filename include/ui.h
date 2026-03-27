#ifndef UI_H
#define UI_H

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "grid.h"
#include "step.h"

typedef struct {
    WINDOW *grid_win;
    WINDOW *info_win;
} Ui;

typedef enum {
    CP_NORMAL,
    CP_CLUE,
    CP_TRIGGER,
    CP_REMOVAL
} ColorPair;

typedef enum {
    ACTION_QUIT,
    ACTION_NEXT
} InputAction;

void ui_init(Ui *ui);
void ui_deinit(Ui *ui);
InputAction ui_wait_for_input(void);
void ui_print_message(Ui *ui, char *format, ...)
    __attribute__((format(printf, 2, 3)));
void ui_print_grid(Ui *ui, Grid *grid, Step *step);

#endif
