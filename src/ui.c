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
#include "util/dynarr.h"
#include "util/dynstr.h"

#define GRID_WIDTH 91
#define GRID_HEIGHT 37

#define INFO_WIDTH (COLS - SCROLLBAR_WIDTH - 1)
#define INFO_HEIGHT (LINES - GRID_HEIGHT)

#define SCROLLBAR_WIDTH 1
#define SCROLLBAR_HEIGHT (LINES - GRID_HEIGHT)

static int color_attr(ColorPair color);
static void generate_lines(DynStr *buf, Lines *lines);
static void refresh_info(Ui *ui);
static void print_scroll_indicators(Ui *ui);

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
    init_pair(CP_TRIGGER, COLOR_GREEN, -1);  // Reverse
    init_pair(CP_REMOVAL, COLOR_RED, -1);    // Reverse
    init_pair(CP_SPECIAL, COLOR_YELLOW, -1); // Reverse

    ui->grid_win = newwin(GRID_HEIGHT, GRID_WIDTH, 0, (COLS - GRID_WIDTH) / 2);
    ui->info_win = newwin(INFO_HEIGHT, INFO_WIDTH, GRID_HEIGHT, 0);
    ui->scrollbar_win = newwin(SCROLLBAR_HEIGHT, SCROLLBAR_WIDTH, GRID_HEIGHT,
                               COLS - SCROLLBAR_WIDTH);
    ds_init(&ui->info_buf);
    da_init(&ui->lines);

    refresh();
}

void ui_deinit(Ui *ui) {
    delwin(ui->grid_win);
    delwin(ui->info_win);
    delwin(ui->scrollbar_win);
    ds_deinit(&ui->info_buf);
    da_deinit(&ui->lines);
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
        case 'p':
        case KEY_BACKSPACE:
        case KEY_LEFT: return ACTION_PREV;
        case 'j':
        case KEY_DOWN: return ACTION_SCROLL_DOWN;
        case 'k':
        case KEY_UP: return ACTION_SCROLL_UP;
        }
    }
}

void ui_scroll(Ui *ui, int offset) {
    ui->curr_line += offset;
    if (ui->curr_line > ui->lines.len - INFO_HEIGHT) {
        ui->curr_line = ui->lines.len - INFO_HEIGHT;
    }
    if (ui->curr_line < 0) {
        ui->curr_line = 0;
    }
    refresh_info(ui);
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

void ui_print_step(Ui *ui, Step *step) {
    ds_clear(&ui->info_buf);
    da_clear(&ui->lines);

    if (step) {
        technique_ops[step->type].explain(&ui->info_buf, step);
        generate_lines(&ui->info_buf, &ui->lines);
    }

    ui->curr_line = 0;
    refresh_info(ui);
}

static int color_attr(ColorPair color) {
    int attr = COLOR_PAIR(color);
    if (color == CP_TRIGGER || color == CP_REMOVAL || color == CP_SPECIAL) {
        attr |= A_REVERSE;
    }
    return attr;
}

static void generate_lines(DynStr *buf, Lines *lines) {
    int last_start = 0;
    for (int i = 0; i < buf->len; i++) {
        int line_len = i - last_start + 1;
        if (buf->elems[i] != '\n' && line_len < INFO_WIDTH) continue;

        Line line = {
            .start = last_start,
            .len = buf->elems[i] == '\n' ? line_len - 1 : line_len,
        };

        if (line.len > 0) {
            da_append(lines, line);
        }
        last_start = i + 1;
    }

    if (last_start < buf->len) {
        da_append(lines, ((Line){last_start, buf->len - last_start}));
    }
}

static void refresh_info(Ui *ui) {
    wclear(ui->info_win);
    for (int i = ui->curr_line;
         i < ui->lines.len && i - ui->curr_line < INFO_HEIGHT; i++) {
        Line line = ui->lines.elems[i];
        wprintw(ui->info_win, "%.*s", line.len,
                &ui->info_buf.elems[line.start]);
        if (line.len < INFO_WIDTH) {
            wprintw(ui->info_win, "\n");
        }
    }
    wrefresh(ui->info_win);
    print_scroll_indicators(ui);
}

static void print_scroll_indicators(Ui *ui) {
    wclear(ui->scrollbar_win);
    if (ui->curr_line > 0) {
        mvwaddwstr(ui->scrollbar_win, 0, 0, L"↑");
    }
    if (ui->lines.len - ui->curr_line > INFO_HEIGHT) {
        mvwaddwstr(ui->scrollbar_win, SCROLLBAR_HEIGHT - 1, 0, L"↓");
    }
    wrefresh(ui->scrollbar_win);
}
