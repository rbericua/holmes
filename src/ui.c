#include "ui.h"

#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "cand_set.h"
#include "cell.h"
#include "grid.h"
#include "step.h"
#include "techniques/registry.h"

#define GRID_WIDTH 91
#define GRID_HEIGHT 37
#define INFO_WIDTH (COLS - 2)
#define INFO_HEIGHT (LINES - GRID_HEIGHT)

static void ui_explain_step(Ui *ui, Step *step);
static void ui_print_scroll_indicators(Ui *ui);
static void ui_refresh_info(Ui *ui);
static void generate_colors(ColorPair colors[81][9], Step *step);

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
    init_pair(CP_REMOVAL, COLOR_BLACK, COLOR_RED);
    init_pair(CP_TRIGGER, COLOR_BLACK, COLOR_GREEN);

    ui->grid_win = newwin(GRID_HEIGHT, GRID_WIDTH, 0, (COLS - GRID_WIDTH) / 2);
    ui->info_win = newpad(LINES, INFO_WIDTH);
    ui->scroll_win = newwin(INFO_HEIGHT, 1, GRID_HEIGHT, COLS - 1);

    refresh();
}

void ui_deinit(Ui *ui) {
    delwin(ui->grid_win);
    delwin(ui->info_win);
    delwin(ui->scroll_win);
    endwin();
}

void ui_print_message(Ui *ui, bool do_clear, bool do_refresh, char *format,
                      ...) {
    va_list args;
    va_start(args, format);

    if (do_clear) {
        wclear(ui->info_win);
    }

    vw_printw(ui->info_win, format, args);

    ui->curr_line = 0;
    ui->max_line = getcury(ui->info_win) - 1;

    if (do_refresh) {
        ui_refresh_info(ui);
    }

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

    ColorPair colors[81][9] = {0};
    generate_colors(colors, step);

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

                int idx = IDX_FROM_ROW_COL(row, col);
                Cell *cell = grid->cells[idx];
                if (cell_is_empty(cell)) {
                    wprintw(ui->grid_win, " ");
                    for (int cand = subrow * 3 + 1; cand <= subrow * 3 + 3;
                         cand++) {
                        if (cell_has_cand(cell, cand)) {
                            ColorPair color = colors[idx][cand - 1];
                            wattron(ui->grid_win, COLOR_PAIR(color));
                            wprintw(ui->grid_win, "%d", cand);
                            wattroff(ui->grid_win, COLOR_PAIR(color));
                        } else {
                            wprintw(ui->grid_win, " ");
                        }

                        if (cand != subrow * 3 + 3) {
                            wprintw(ui->grid_win, "  ");
                        }
                    }
                    wprintw(ui->grid_win, " ");
                } else {
                    if (subrow == 1) {
                        if (cell->is_clue) {
                            wattron(ui->grid_win, COLOR_PAIR(CP_CLUE));
                        }
                        wprintw(ui->grid_win, "    %d    ", cell->value);
                        wattroff(ui->grid_win, COLOR_PAIR(CP_CLUE));
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

void ui_print_step(Ui *ui, Step *step) {
    wclear(ui->info_win);

    ui_explain_step(ui, step);

    ui->curr_line = 0;
    ui->max_line = getcury(ui->info_win) - 1;
    ui_refresh_info(ui);
}

void ui_print_cand_set(Ui *ui, CandSet set) {
    int arr[9];
    cand_set_to_arr(set, arr);

    wprintw(ui->info_win, "{");
    for (int i = 0; i < set.len; i++) {
        wprintw(ui->info_win, "%d", arr[i]);
        if (i < set.len - 1) {
            wprintw(ui->info_win, ", ");
        }
    }
    wprintw(ui->info_win, "}");
}

void ui_print_idxs(Ui *ui, int idxs[], int num_idxs) {
    for (int i = 0; i < num_idxs; i++) {
        int row = ROW_FROM_IDX(idxs[i]);
        int col = COL_FROM_IDX(idxs[i]);

        wprintw(ui->info_win, "r%dc%d", row + 1, col + 1);
        if (i < num_idxs - 1) {
            wprintw(ui->info_win, ", ");
        }
    }
}

void ui_scroll(Ui *ui, int delta) {
    ui->curr_line += delta;
    if (ui->curr_line > ui->max_line - INFO_HEIGHT + 1) {
        ui->curr_line = ui->max_line - INFO_HEIGHT + 1;
    }
    if (ui->curr_line < 0) {
        ui->curr_line = 0;
    }

    ui_refresh_info(ui);
}

InputAction ui_wait_for_input(void) {
    while (true) {
        switch (getch()) {
        case 'q': return ACTION_QUIT;
        case 'p':
        case KEY_BACKSPACE:
        case KEY_LEFT: return ACTION_PREV;
        case 'n':
        case ' ':
        case '\n':
        case KEY_RIGHT: return ACTION_NEXT;
        case 'k':
        case KEY_UP: return ACTION_SCROLL_UP;
        case 'j':
        case KEY_DOWN: return ACTION_SCROLL_DOWN;
        }
    }
}

static void ui_explain_step(Ui *ui, Step *step) {
    if (!step) return;
    technique_ops[step->tech].explain(ui, step);
}

static void ui_print_scroll_indicators(Ui *ui) {
    wmove(ui->scroll_win, 0, 0);
    if (ui->curr_line > 0) {
        waddwstr(ui->scroll_win, L"↑");
    } else {
        wprintw(ui->scroll_win, " ");
    }

    wmove(ui->scroll_win, getmaxy(ui->scroll_win) - 1, 0);
    if (ui->max_line - ui->curr_line + 1 > INFO_HEIGHT) {
        waddwstr(ui->scroll_win, L"↓");
    } else {
        wprintw(ui->scroll_win, " ");
    }

    wrefresh(ui->scroll_win);
}

static void ui_refresh_info(Ui *ui) {
    prefresh(ui->info_win, ui->curr_line, 0, GRID_HEIGHT, 0, LINES - 1,
             INFO_WIDTH - 1);
    ui_print_scroll_indicators(ui);
}

static void generate_colors(ColorPair colors[81][9], Step *step) {
    if (!step) return;
    technique_ops[step->tech].colorise(colors, step);
}
