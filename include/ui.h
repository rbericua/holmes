#ifndef UI_H
#define UI_H

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "grid.h"
#include "step.h"
#include "util/dynstr.h"

typedef struct {
    int start;
    int len;
} Line;

typedef struct {
    Line *elems;
    int len;
    int cap;
} Lines;

typedef struct {
    WINDOW *grid_win;
    WINDOW *info_win;
    WINDOW *scrollbar_win;
    DynStr info_buf;
    Lines lines;
    int curr_line;
} Ui;

typedef enum {
    CP_NORMAL,
    CP_CLUE,

    CP_REVERSE_START,

    CP_TRIGGER = CP_REVERSE_START,
    CP_REMOVAL,
    CP_SPECIAL1,
    CP_SPECIAL2
} ColorPair;

typedef enum {
    ACTION_QUIT,
    ACTION_NEXT,
    ACTION_PREV,
    ACTION_SCROLL_DOWN,
    ACTION_SCROLL_UP
} InputAction;

void ui_init(Ui *ui);
void ui_deinit(Ui *ui);
InputAction ui_wait_for_input(void);
void ui_scroll(Ui *ui, int offset);
void ui_print_message(Ui *ui, char *format, ...)
    __attribute__((format(printf, 2, 3)));
void ui_print_grid(Ui *ui, Grid *grid, Step *step);
void ui_print_step(Ui *ui, Step *step);

#endif
