#include "ui.h"

#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "cell.h"
#include "dynarr.h"
#include "dynstr.h"
#include "grid.h"
#include "step.h"
#include "techniques/registry.h"

#define GRID_WIDTH 91
#define GRID_HEIGHT 37
#define INFO_WIDTH (COLS - 2)
#define INFO_HEIGHT (LINES - GRID_HEIGHT)

static void ui_explain_step(Ui *ui, Step *step);
static void ui_refresh_info(Ui *ui);
static void ui_print_scroll_indicators(Ui *ui);
static void ui_generate_lines(Ui *ui);
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
    ui->info_win = newwin(INFO_HEIGHT, INFO_WIDTH + 1, GRID_HEIGHT, 0);
    ui->scroll_win = newwin(INFO_HEIGHT, 1, GRID_HEIGHT, COLS - 1);
    ds_init(&ui->info_buf);
    da_init(&ui->lines);

    refresh();
}

void ui_deinit(Ui *ui) {
    delwin(ui->grid_win);
    delwin(ui->info_win);
    delwin(ui->scroll_win);
    ds_deinit(&ui->info_buf);
    da_deinit(&ui->lines);
    endwin();
}

void ui_print_message(Ui *ui, char *format, ...) {
    wclear(ui->info_win);

    va_list args;
    va_start(args, format);

    ds_clear(&ui->info_buf);
    ds_vappendf(&ui->info_buf, format, args);
    ui_generate_lines(ui);

    va_end(args);

    ui->curr_line = 0;
    ui_refresh_info(ui);
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
    ui_explain_step(ui, step);
    ui_generate_lines(ui);
    ui->curr_line = 0;
    ui_refresh_info(ui);
}

void ui_scroll(Ui *ui, int delta) {
    ui->curr_line += delta;
    if (ui->curr_line > ui->lines.len - INFO_HEIGHT) {
        ui->curr_line = ui->lines.len - INFO_HEIGHT;
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
    ds_clear(&ui->info_buf);
    technique_ops[step->tech].explain(&ui->info_buf, step);
}

static void ui_refresh_info(Ui *ui) {
    int num_lines;
    if (ui->lines.len < INFO_HEIGHT) {
        num_lines = ui->lines.len;
    } else {
        num_lines = INFO_HEIGHT;
    }

    wclear(ui->info_win);
    for (int i = 0; i < num_lines; i++) {
        Line curr_line = ui->lines.elems[ui->curr_line + i];

        wprintw(ui->info_win, "%.*s\n", curr_line.len,
                ui->info_buf.elems + curr_line.start);
    }
    wrefresh(ui->info_win);
    ui_print_scroll_indicators(ui);
}

static void ui_print_scroll_indicators(Ui *ui) {
    wmove(ui->scroll_win, 0, 0);
    if (ui->curr_line > 0) {
        waddwstr(ui->scroll_win, L"↑");
    } else {
        wprintw(ui->scroll_win, " ");
    }

    wmove(ui->scroll_win, getmaxy(ui->scroll_win) - 1, 0);
    if (ui->lines.len - ui->curr_line > INFO_HEIGHT) {
        waddwstr(ui->scroll_win, L"↓");
    } else {
        wprintw(ui->scroll_win, " ");
    }

    wrefresh(ui->scroll_win);
}

static void ui_generate_lines(Ui *ui) {
    int line_start = 0;
    int line_len = 0;

    da_clear(&ui->lines);
    for (int i = 0; i < ui->info_buf.len; i++) {
        if (ui->info_buf.elems[i] != '\n' && line_len != INFO_WIDTH) {
            line_len++;
            continue;
        }

        Line line = {.start = line_start, .len = line_len};
        da_append(&ui->lines, line);

        line_start = line_len == INFO_WIDTH ? i : i + 1;
        line_len = 0;
    }
}

static void generate_colors(ColorPair colors[81][9], Step *step) {
    if (!step) return;
    technique_ops[step->tech].colorise(colors, step);
}
