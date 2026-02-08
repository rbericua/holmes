#include "ui.h"

#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>

#define NCURSES_WIDECHAR 1
#include <ncurses.h>

#include "cell.h"
#include "grid.h"
#include "step.h"

#define GRID_WIDTH 91
#define GRID_HEIGHT 37

#define UNIT_TO_STR(u) \
    ((u) == UNIT_ROW ? "Row" : (u) == UNIT_COL ? "Column" : "Box")

static void generate_colors(Grid *grid, Step *step, ColorPair colors[81][9]);

void ui_init(Ui *ui) {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    start_color();
    use_default_colors();
    init_pair(CP_REMOVAL, COLOR_BLACK, COLOR_RED);
    init_pair(CP_TRIGGER, COLOR_BLACK, COLOR_GREEN);

    ui->grid_win = newwin(GRID_HEIGHT, GRID_WIDTH, 0, (COLS - GRID_WIDTH) / 2);
    ui->info_win = newwin(LINES - GRID_HEIGHT, COLS, GRID_HEIGHT, 0);

    refresh();
}

void ui_deinit(Ui *ui) {
    delwin(ui->grid_win);
    delwin(ui->info_win);
    endwin();
}

void ui_print_message(Ui *ui, bool clear, char *format, ...) {
    va_list args;
    va_start(args, format);

    if (clear) {
        wclear(ui->info_win);
    }
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

    ColorPair colors[81][9] = {0};
    if (step) {
        generate_colors(grid, step, colors);
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

                int idx = IDX_FROM_ROW_COL(row, col);
                Cell *cell = grid->cells[idx];
                if (cell_is_empty(cell)) {
                    waddstr(ui->grid_win, " ");
                    for (int cand = subrow * 3 + 1; cand <= subrow * 3 + 3;
                         cand++) {
                        if (cell_has_cand(cell, cand)) {
                            ColorPair color = colors[idx][cand - 1];
                            wattron(ui->grid_win, COLOR_PAIR(color));
                            wprintw(ui->grid_win, "%d", cand);
                            wattroff(ui->grid_win, COLOR_PAIR(color));
                        } else {
                            waddstr(ui->grid_win, " ");
                        }

                        if (cand != subrow * 3 + 3) {
                            waddstr(ui->grid_win, "  ");
                        }
                    }
                    waddstr(ui->grid_win, " ");
                } else {
                    if (subrow == 1) {
                        wprintw(ui->grid_win, "    %d    ", cell->value);
                    } else {
                        waddstr(ui->grid_win, "         ");
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
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        int row = ROW_FROM_IDX(step->as.naked_single.idx);
        int col = COL_FROM_IDX(step->as.naked_single.idx);
        int value = step->as.naked_single.value;

        ui_print_message(ui, true, "[Naked Single] Set r%dc%d to %d\n", row + 1,
                         col + 1, value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        int row = ROW_FROM_IDX(step->as.hidden_single.idx);
        int col = COL_FROM_IDX(step->as.hidden_single.idx);
        int value = step->as.hidden_single.value;
        char *unit_str = UNIT_TO_STR(step->as.hidden_single.unit_type);
        int unit_idx = step->as.hidden_single.unit_idx;

        ui_print_message(ui, true, "[Hidden Single (%s %d)] Set r%dc%d to %d\n",
                         unit_str, unit_idx + 1, row + 1, col + 1, value);
    } break;
    default: break;
    }
}

static void generate_colors(Grid *grid, Step *step, ColorPair colors[81][9]) {
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        int idx = step->as.naked_single.idx;
        int value = step->as.naked_single.value;

        colors[idx][value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_NEIGHBOURS; i++) {
            int neighbour_idx = cell_idx(grid->neighbours[idx][i]);
            colors[neighbour_idx][value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_HIDDEN_SINGLE: {
        int idx = step->as.hidden_single.idx;
        int value = step->as.hidden_single.value;

        colors[idx][value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_NEIGHBOURS; i++) {
            int neighbour_idx = cell_idx(grid->neighbours[idx][i]);
            colors[neighbour_idx][value - 1] = CP_REMOVAL;
        }
    } break;
    default: break;
    }
}
