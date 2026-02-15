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
#define UNIT_TO_STR_PLURAL(u) \
    ((u) == UNIT_ROW ? "Rows" : (u) == UNIT_COL ? "Columns" : "Boxs")
#define SET_NAME_FROM_SIZE(n) ((n) == 2 ? "Pair" : (n) == 3 ? "Triple" : "Quad")
#define FISH_NAME_FROM_SIZE(n) \
    ((n) == 2 ? "X-Wing" : (n) == 3 ? "Swordfish" : "Jellyfish")

static void generate_colors(Step *step, ColorPair colors[81][9]);

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
        NakedSingleStep *s = &step->as.naked_single;

        int row = ROW_FROM_IDX(s->idx);
        int col = COL_FROM_IDX(s->idx);

        ui_print_message(ui, true, "[Naked Single] Set r%dc%d to %d\n", row + 1,
                         col + 1, s->value);
    } break;
    case TECH_HIDDEN_SINGLE: {
        HiddenSingleStep *s = &step->as.hidden_single;

        int row = ROW_FROM_IDX(s->idx);
        int col = COL_FROM_IDX(s->idx);
        char *unit_str = UNIT_TO_STR(s->unit_type);

        ui_print_message(ui, true, "[Hidden Single (%s %d)] Set r%dc%d to %d\n",
                         unit_str, s->unit_idx + 1, row + 1, col + 1, s->value);
    } break;
    case TECH_NAKED_PAIR:
    case TECH_NAKED_TRIPLE:
    case TECH_NAKED_QUAD: {
        NakedSetStep *s = &step->as.naked_set;

        char *unit_str = UNIT_TO_STR(s->unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        ui_print_message(ui, true, "[Naked %s (%s %d)] ", set_name, unit_str,
                         s->unit_idx + 1);
        ui_print_cand_set(ui, s->cands);
        ui_print_message(ui, false, " in ");
        ui_print_idxs(ui, s->idxs, s->size);
        ui_print_message(ui, false, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            ui_print_message(ui, false, "- Removed ");
            ui_print_cand_set(ui, s->removed_cands[i]);
            ui_print_message(ui, false, " from r%dc%d\n", row + 1, col + 1);
        }
    } break;
    case TECH_HIDDEN_PAIR:
    case TECH_HIDDEN_TRIPLE:
    case TECH_HIDDEN_QUAD: {
        HiddenSetStep *s = &step->as.hidden_set;

        char *unit_str = UNIT_TO_STR(s->unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        ui_print_message(ui, true, "[Hidden %s (%s %d)] ", set_name, unit_str,
                         s->unit_idx + 1);
        ui_print_cand_set(ui, s->cands);
        ui_print_message(ui, false, " in ");
        ui_print_idxs(ui, s->idxs, s->size);
        ui_print_message(ui, false, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            ui_print_message(ui, false, "- Removed ");
            ui_print_cand_set(ui, s->removed_cands[i]);
            ui_print_message(ui, false, " from r%dc%d\n", row + 1, col + 1);
        }

    }; break;
    case TECH_POINTING_SET: {
        PointingSetStep *s = &step->as.pointing_set;

        char *trigger_unit_str = UNIT_TO_STR(s->trigger_unit_type);
        char *removal_unit_str = UNIT_TO_STR(s->removal_unit_type);
        char *set_name = SET_NAME_FROM_SIZE(s->size);

        ui_print_message(ui, true, "[Pointing %s (%s %d -> %s %d)] {%d} in ",
                         set_name, trigger_unit_str, s->trigger_unit_idx + 1,
                         removal_unit_str, s->removal_unit_idx + 1, s->value);
        ui_print_idxs(ui, s->idxs, s->size);
        ui_print_message(ui, false, ":\n");
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            ui_print_message(ui, false, "- Removed {%d} from r%dc%d\n",
                             s->value, row + 1, col + 1);
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

        ui_print_message(ui, true, "[%s (%s ", fish_name, base_str);
        for (int i = 0; i < s->size; i++) {
            ui_print_message(ui, false, "%d", s->base_idxs[i] + 1);
            if (i < s->size - 1) {
                ui_print_message(ui, false, ", ");
            }
        }
        ui_print_message(ui, false, " -> %s ", cover_str);
        for (int i = 0; i < s->size; i++) {
            ui_print_message(ui, false, "%d", s->cover_idxs[i] + 1);
            if (i < s->size - 1) {
                ui_print_message(ui, false, ", ");
            }
        }
        ui_print_message(ui, false, "] {%d}:\n", s->value);
        for (int i = 0; i < s->num_removals; i++) {
            int row = ROW_FROM_IDX(s->removal_idxs[i]);
            int col = COL_FROM_IDX(s->removal_idxs[i]);

            ui_print_message(ui, false, "- Removed {%d} from r%dc%d\n",
                             s->value, row + 1, col + 1);
        }
    }
    default: break;
    }
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
