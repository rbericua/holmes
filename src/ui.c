#include "ui.h"

#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "geometry.h"
#include "grid.h"
#include "step.h"
#include "techniques/registry.h"

#define GRID_WIDTH 91
#define GRID_HEIGHT 37

static int color_attr(ColorPair color);

void ui_init(Ui *ui) {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, true);

    start_color();
    use_default_colors();
    init_pair(CP_CLUE, COLOR_BLUE, -1);
    init_pair(CP_TRIGGER, COLOR_GREEN, -1); // Reverse
    init_pair(CP_REMOVAL, COLOR_RED, -1);   // Reverse

    ui->grid_win = newwin(GRID_HEIGHT, GRID_WIDTH, 0, (COLS - GRID_WIDTH) / 2);
    ui->info_win = newwin(LINES - GRID_HEIGHT, COLS, GRID_HEIGHT, 0);

    refresh();
}

void ui_deinit(Ui *ui) {
    delwin(ui->grid_win);
    delwin(ui->info_win);
    endwin();
}

InputAction ui_wait_for_input(void) {
    while (true) {
        switch (getch()) {
        case 'q': return ACTION_QUIT;
        case 'n':
        case ' ':
        case '\n':
        case KEY_RIGHT: return ACTION_NEXT;
        }
    }
}

void ui_print_message(Ui *ui, char *format, ...) {
    va_list args;
    va_start(args, format);

    wclear(ui->info_win);
    vw_printw(ui->info_win, format, args);
    wrefresh(ui->info_win);

    va_end(args);
}

void ui_print_grid(Ui *ui, Grid *grid, Step *step) {
    wchar_t *top = L"┏━━━━━━━━━┯━━━━━━━━━┯━━━━━━━━━┳━━━━━━━━━┯━━━━━━━━━┯━━━━━━━"
                   L"━━┳━━━━━━━━━┯━━━━━━━━━┯━━━━━━━━━┓";
    wchar_t *row_sep = L"┠─────────┼─────────┼─────────╂─────────┼─────────┼───"
                       L"──────╂─────────┼─────────┼─────────┨";
    wchar_t *band_sep = L"┣━━━━━━━━━┿━━━━━━━━━┿━━━━━━━━━╋━━━━━━━━━┿━━━━━━━━━┿━━"
                        L"━━━━━━━╋━━━━━━━━━┿━━━━━━━━━┿━━━━━━━━━┫";
    wchar_t *bottom = L"┗━━━━━━━━━┷━━━━━━━━━┷━━━━━━━━━┻━━━━━━━━━┷━━━━━━━━━┷━━━━"
                      L"━━━━━┻━━━━━━━━━┷━━━━━━━━━┷━━━━━━━━━┛";
    ColorPair colors[81][9];
    if (step) {
        technique_ops[step->type].colorise(colors, step);
    }

    wclear(ui->grid_win);
    for (int row = 0; row < 9; row++) {
        if (row == 0) {
            waddwstr(ui->grid_win, top);
        } else if (row % 3 == 0) {
            waddwstr(ui->grid_win, band_sep);
        } else {
            waddwstr(ui->grid_win, row_sep);
        }

        for (int subrow = 0; subrow < 3; subrow++) {
            for (int col = 0; col < 9; col++) {
                if (col % 3 == 0) {
                    waddwstr(ui->grid_win, L"┃");
                } else {
                    waddwstr(ui->grid_win, L"│");
                }

                int cell = cell_from_row_col(row, col);
                if (grid_cell_is_empty(grid, cell)) {
                    wprintw(ui->grid_win, " ");
                    for (int cand_i = 0; cand_i < 3; cand_i++) {
                        int cand = subrow * 3 + cand_i + 1;

                        ColorPair color = colors[cell][cand - 1];

                        if (grid_cell_has_cand(grid, cell, cand)) {
                            wattron(ui->grid_win, color_attr(color));
                            wprintw(ui->grid_win, "%d", cand);
                            wattroff(ui->grid_win, color_attr(color));
                        } else {
                            wprintw(ui->grid_win, " ");
                        }
                        if (cand_i != 2) {
                            wprintw(ui->grid_win, "  ");
                        }
                    }
                    wprintw(ui->grid_win, " ");
                } else {
                    if (subrow == 1) {
                        if (grid_cell_is_clue(grid, cell)) {
                            wattron(ui->grid_win, color_attr(CP_CLUE));
                        }
                        wprintw(ui->grid_win, "    %d    ",
                                grid_cell_value(grid, cell));
                        wattroff(ui->grid_win, color_attr(CP_CLUE));
                    } else {
                        wprintw(ui->grid_win, "         ");
                    }
                }
            }
            waddwstr(ui->grid_win, L"┃");
        }
    }
    waddwstr(ui->grid_win, bottom);
    wrefresh(ui->grid_win);
}

static int color_attr(ColorPair color) {
    int attr = COLOR_PAIR(color);
    if (color == CP_TRIGGER || color == CP_REMOVAL) {
        attr |= A_REVERSE;
    }
    return attr;
}
