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

#define GRID_WIDTH 91
#define GRID_HEIGHT 37

#define UNIT_TO_STR(u) \
    ((u) == UNIT_ROW ? "Row" : (u) == UNIT_COL ? "Column" : "Box")
#define SET_NAME_FROM_SIZE(n) ((n) == 2 ? "Pair" : (n) == 3 ? "Triple" : "Quad")

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

void ui_print_cand_set(Ui *ui, CandSet set) {
    int arr[9];
    cand_set_to_arr(set, arr);

    ui_print_message(ui, false, "{");
    for (int i = 0; i < set.len; i++) {
        ui_print_message(ui, false, "%d", arr[i]);
        if (i < set.len - 1) {
            ui_print_message(ui, false, ", ");
        }
    }
    ui_print_message(ui, false, "}");
}

void ui_print_idxs(Ui *ui, int idxs[], int num_idxs) {
    for (int i = 0; i < num_idxs; i++) {
        int row = ROW_FROM_IDX(idxs[i]);
        int col = COL_FROM_IDX(idxs[i]);

        ui_print_message(ui, false, "r%dc%d", row + 1, col + 1);
        if (i < num_idxs - 1) {
            ui_print_message(ui, false, ", ");
        }
    }
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
        NakedSingleStep *naked_single = &step->as.naked_single;

        int row = ROW_FROM_IDX(naked_single->idx);
        int col = COL_FROM_IDX(naked_single->idx);

        ui_print_message(ui, true, "[Naked Single] Set r%dc%d to %d\n", row + 1,
                         col + 1, naked_single->value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *hidden_single = &step->as.hidden_single;

        int row = ROW_FROM_IDX(hidden_single->idx);
        int col = COL_FROM_IDX(hidden_single->idx);
        char *unit_str = UNIT_TO_STR(hidden_single->unit_type);

        ui_print_message(ui, true, "[Hidden Single (%s %d)] Set r%dc%d to %d\n",
                         unit_str, hidden_single->unit_idx + 1, row + 1,
                         col + 1, hidden_single->value);
    } break;
    case TECH_NAKED_SET: {
        NakedSetStep *naked_set = &step->as.naked_set;

        char *unit_str = UNIT_TO_STR(naked_set->unit_type);
        char *set_name = SET_NAME_FROM_SIZE(naked_set->set_size);

        ui_print_message(ui, true, "[Naked %s (%s %d)] ", set_name, unit_str,
                         naked_set->unit_idx + 1);
        ui_print_cand_set(ui, naked_set->set_cands);
        ui_print_message(ui, false, " in ");
        ui_print_idxs(ui, naked_set->set_idxs, naked_set->set_size);
        ui_print_message(ui, false, ":\n");
        for (int i = 0; i < naked_set->num_removals; i++) {
            int row = ROW_FROM_IDX(naked_set->removal_idxs[i]);
            int col = COL_FROM_IDX(naked_set->removal_idxs[i]);

            ui_print_message(ui, false, "- Removed ");
            ui_print_cand_set(ui, naked_set->removed_cands[i]);
            ui_print_message(ui, false, " from r%dc%d\n", row + 1, col + 1);
        }
    } break;
    default: break;
    }
}

static void generate_colors(Grid *grid, Step *step, ColorPair colors[81][9]) {
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        NakedSingleStep *naked_single = &step->as.naked_single;

        colors[naked_single->idx][naked_single->value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_PEERS; i++) {
            int peer_idx = cell_idx(grid->peers[naked_single->idx][i]);
            colors[peer_idx][naked_single->value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *hidden_single = &step->as.hidden_single;

        colors[hidden_single->idx][hidden_single->value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_PEERS; i++) {
            int peer_idx = cell_idx(grid->peers[hidden_single->idx][i]);
            colors[peer_idx][hidden_single->value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_NAKED_SET: {
        NakedSetStep *naked_set = &step->as.naked_set;

        int cands[4];
        cand_set_to_arr(naked_set->set_cands, cands);

        for (int i = 0; i < naked_set->set_size; i++) {
            int cand = cands[i];
            for (int j = 0; j < naked_set->set_size; j++) {
                colors[naked_set->set_idxs[j]][cand - 1] = CP_TRIGGER;
            }
            for (int j = 0; j < naked_set->num_removals; j++) {
                colors[naked_set->removal_idxs[j]][cand - 1] = CP_REMOVAL;
            }
        }
    }; break;
    default: break;
    }
}
