#ifndef UI_H
#define UI_H

#include <stdbool.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "grid.h"
#include "step.h"

typedef struct {
    WINDOW *grid_win;
    WINDOW *info_win;
    WINDOW *scroll_win;
    int curr_line;
    int max_line;
} Ui;

typedef enum {
    CP_DEFAULT,
    CP_CLUE,
    CP_REMOVAL,
    CP_TRIGGER,
} ColorPair;

typedef enum {
    ACTION_QUIT,
    ACTION_PREV,
    ACTION_NEXT,
    ACTION_SCROLL_UP,
    ACTION_SCROLL_DOWN,
} InputAction;

void ui_init(Ui *ui);
void ui_deinit(Ui *ui);
void ui_print_message(Ui *ui, char *format, ...);
void ui_print_grid(Ui *ui, Grid *grid, Step *step);
void ui_print_step(Ui *ui, Step *step);
void ui_scroll(Ui *ui, int delta);
InputAction ui_wait_for_input(void);

#endif
