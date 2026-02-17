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
#define INFO_WIDTH (COLS - 2)
#define INFO_HEIGHT (LINES - GRID_HEIGHT)

#define UNIT_TO_STR(u) \
    ((u) == UNIT_ROW ? "Row" : (u) == UNIT_COL ? "Column" : "Box")
#define UNIT_TO_STR_PLURAL(u) \
    ((u) == UNIT_ROW ? "Rows" : (u) == UNIT_COL ? "Columns" : "Boxs")
#define SET_NAME_FROM_SIZE(n) ((n) == 2 ? "Pair" : (n) == 3 ? "Triple" : "Quad")
#define FISH_NAME_FROM_SIZE(n) \
    ((n) == 2 ? "X-Wing" : (n) == 3 ? "Swordfish" : "Jellyfish")

static void ui_print_cand_set(Ui *ui, CandSet set);
static void ui_print_idxs(Ui *ui, int idxs[], int num_idxs);
static void ui_print_scroll_indicators(Ui *ui);
static void ui_refresh_info(Ui *ui);
static void generate_colors(Step *step, ColorPair colors[81][9]);

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

void ui_print_message(Ui *ui, char *format, ...) {
    va_list args;
    va_start(args, format);

    wclear(ui->info_win);
    vw_printw(ui->info_win, format, args);
    ui_refresh_info(ui);

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
        generate_colors(step, colors);
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

    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        NakedSingleStep *s = &step->as.naked_single;

        int row = ROW_FROM_IDX(s->idx);
        int col = COL_FROM_IDX(s->idx);

        wprintw(ui->info_win, "[Naked Single] Set r%dc%d to %d\n", row + 1,
                col + 1, s->value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *s = &step->as.hidden_single;

        int row = ROW_FROM_IDX(s->idx);
        int col = COL_FROM_IDX(s->idx);
        char *unit_str = UNIT_TO_STR(s->unit_type);

        wprintw(ui->info_win, "[Hidden Single (%s %d)] Set r%dc%d to %d\n",
                unit_str, s->unit_idx + 1, row + 1, col + 1, s->value);
    } break;
    case TECH_NAKED_PAIR:
    case TECH_NAKED_TRIPLE:
    case TECH_NAKED_QUAD: {
        NakedSetStep *s = &step->as.naked_set;

        char *unit_str = UNIT_TO_STR(s->unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        wprintw(ui->info_win, "[Naked %s (%s %d)] ", set_name, unit_str,
                s->unit_idx + 1);
        ui_print_cand_set(ui, s->cands);
        wprintw(ui->info_win, " in ");
        ui_print_idxs(ui, s->idxs, s->size);
        wprintw(ui->info_win, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            wprintw(ui->info_win, "- Removed ");
            ui_print_cand_set(ui, s->removed_cands[i]);
            wprintw(ui->info_win, " from r%dc%d\n", row + 1, col + 1);
        }
    } break;
    case TECH_HIDDEN_PAIR:
    case TECH_HIDDEN_TRIPLE:
    case TECH_HIDDEN_QUAD: {
        HiddenSetStep *s = &step->as.hidden_set;

        char *unit_str = UNIT_TO_STR(s->unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        wprintw(ui->info_win, "[Hidden %s (%s %d)] ", set_name, unit_str,
                s->unit_idx + 1);
        ui_print_cand_set(ui, s->cands);
        wprintw(ui->info_win, " in ");
        ui_print_idxs(ui, s->idxs, s->size);
        wprintw(ui->info_win, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            wprintw(ui->info_win, "- Removed ");
            ui_print_cand_set(ui, s->removed_cands[i]);
            wprintw(ui->info_win, " from r%dc%d\n", row + 1, col + 1);
        }

    }; break;
    case TECH_POINTING_SET: {
        PointingSetStep *s = &step->as.pointing_set;

        char *trigger_unit_str = UNIT_TO_STR(s->trigger_unit_type);
        char *removal_unit_str = UNIT_TO_STR(s->removal_unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        wprintw(ui->info_win, "[Pointing %s (%s %d -> %s %d)] {%d} in ",
                set_name, trigger_unit_str, s->trigger_unit_idx + 1,
                removal_unit_str, s->removal_unit_idx + 1, s->value);
        ui_print_idxs(ui, s->idxs, s->size);
        wprintw(ui->info_win, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            wprintw(ui->info_win, "- Removed {%d} from r%dc%d\n", s->value,
                    row + 1, col + 1);
        }
    }; break;
    case TECH_X_WING:
    case TECH_SWORDFISH:
    case TECH_JELLYFISH: {
        BasicFishStep *s = &step->as.basic_fish;

        char *fish_name = FISH_NAME_FROM_SIZE(s->size);
        char *base_str = UNIT_TO_STR_PLURAL(s->unit_type);
        char *cover_str = UNIT_TO_STR_PLURAL(
            s->unit_type == UNIT_ROW ? UNIT_COL : UNIT_ROW);

        wprintw(ui->info_win, "[%s (%s ", fish_name, base_str);
        for (int i = 0; i < s->size; i++) {
            wprintw(ui->info_win, "%d", s->base_idxs[i] + 1);
            if (i < s->size - 1) {
                wprintw(ui->info_win, ", ");
            }
        }
        wprintw(ui->info_win, " -> %s ", cover_str);
        for (int i = 0; i < s->size; i++) {
            wprintw(ui->info_win, "%d", s->cover_idxs[i] + 1);
            if (i < s->size - 1) {
                wprintw(ui->info_win, ", ");
            }
        }
        wprintw(ui->info_win, "] {%d}:\n", s->value);
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            wprintw(ui->info_win, "- Removed {%d} from r%dc%d\n", s->value,
                    row + 1, col + 1);
        }
    }
    default: break;
    }

    ui->curr_line = 0;
    ui->max_line = getcury(ui->info_win) - 1;
    ui_refresh_info(ui);
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
        case 'n':
        case ' ':
        case '\n':
        case KEY_RIGHT: return ACTION_NEXT;
        case 'j':
        case KEY_DOWN: return ACTION_SCROLL_DOWN;
        case 'k':
        case KEY_UP: return ACTION_SCROLL_UP;
        }
    }
}

static void ui_print_cand_set(Ui *ui, CandSet set) {
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

static void ui_print_idxs(Ui *ui, int idxs[], int num_idxs) {
    for (int i = 0; i < num_idxs; i++) {
        int row = ROW_FROM_IDX(idxs[i]);
        int col = COL_FROM_IDX(idxs[i]);

        wprintw(ui->info_win, "r%dc%d", row + 1, col + 1);
        if (i < num_idxs - 1) {
            wprintw(ui->info_win, ", ");
        }
    }
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

static void generate_colors(Step *step, ColorPair colors[81][9]) {
    switch (step->tech) {
    case TECH_NAKED_SINGLE: {
        NakedSingleStep *s = &step->as.naked_single;

        colors[s->idx][s->value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_PEERS; i++) {
            int peer_idx = s->peer_idxs[i];
            colors[peer_idx][s->value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *s = &step->as.hidden_single;

        colors[s->idx][s->value - 1] = CP_TRIGGER;
        for (int i = 0; i < NUM_PEERS; i++) {
            int peer_idx = s->peer_idxs[i];
            colors[peer_idx][s->value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_NAKED_PAIR:
    case TECH_NAKED_TRIPLE:
    case TECH_NAKED_QUAD: {
        NakedSetStep *s = &step->as.naked_set;

        int cands[4];
        cand_set_to_arr(s->cands, cands);

        for (int i = 0; i < s->size; i++) {
            int cand = cands[i];
            for (int j = 0; j < s->size; j++) {
                int idx = s->idxs[j];
                colors[idx][cand - 1] = CP_TRIGGER;
            }
            for (int j = 0; j < s->num_removals; j++) {
                int idx = s->removal_idxs[j];
                colors[idx][cand - 1] = CP_REMOVAL;
            }
        }
    }; break;
    case TECH_HIDDEN_PAIR:
    case TECH_HIDDEN_TRIPLE:
    case TECH_HIDDEN_QUAD: {
        HiddenSetStep *s = &step->as.hidden_set;

        for (int i = 0; i < s->size; i++) {
            int idx = s->idxs[i];
            for (int cand = 1; cand <= 9; cand++) {
                colors[idx][cand - 1] = cand_set_has(s->cands, cand)
                                            ? CP_TRIGGER
                                            : CP_REMOVAL;
            }
        }
    } break;
    case TECH_POINTING_SET: {
        PointingSetStep *s = &step->as.pointing_set;

        for (int i = 0; i < s->size; i++) {
            int idx = s->idxs[i];
            colors[idx][s->value - 1] = CP_TRIGGER;
        }
        for (int i = 0; i < s->num_removals; i++) {
            int idx = s->removal_idxs[i];
            colors[idx][s->value - 1] = CP_REMOVAL;
        }
    } break;
    case TECH_X_WING:
    case TECH_SWORDFISH:
    case TECH_JELLYFISH: {
        BasicFishStep *s = &step->as.basic_fish;

        for (int i = 0; i < s->size; i++) {
            int base = s->base_idxs[i];
            for (int j = 0; j < s->size; j++) {
                int cover = s->cover_idxs[j];
                int idx = s->unit_type == UNIT_ROW
                              ? IDX_FROM_ROW_COL(base, cover)
                              : IDX_FROM_ROW_COL(cover, base);
                colors[idx][s->value - 1] = CP_TRIGGER;
            }
        }
        for (int i = 0; i < s->num_removals; i++) {
            int idx = s->removal_idxs[i];
            colors[idx][s->value - 1] = CP_REMOVAL;
        }
    } break;
    default: break;
    }
}
