#include "geometry.h"

int units[3][9][9];
int peers[81][NUM_PEERS];

void geometry_init(void) {
    for (int i = 0; i < 81; i++) {
        int row = cell_row(i);
        int col = cell_col(i);
        int box = cell_box(i);
        int idx_in_box = cell_idx_in_box(i);

        units[UNIT_ROW][row][col] = i;
        units[UNIT_COL][col][row] = i;
        units[UNIT_BOX][box][idx_in_box] = i;
    }

    for (int i = 0; i < 81; i++) {
        int count = 0;
        for (int j = 0; j < 81; j++) {
            if (cells_are_peers(i, j)) {
                peers[i][count++] = j;
            }
        }
    }
}

int cell_unit(int cell, UnitType unit) {
    switch (unit) {
    case UNIT_ROW: return cell_row(cell);
    case UNIT_COL: return cell_col(cell);
    case UNIT_BOX: return cell_box(cell);
    default: return -1;
    }
}

int cell_row(int cell) {
    return cell / 9;
}

int cell_col(int cell) {
    return cell % 9;
}

int cell_box(int cell) {
    return (cell_row(cell) / 3) * 3 + cell_col(cell) / 3;
}

int cell_idx_in_box(int cell) {
    return (cell_row(cell) % 3) * 3 + cell_col(cell) % 3;
}

int cell_from_row_col(int row, int col) {
    return row * 9 + col;
}

bool cells_are_peers(int a, int b) {
    return a != b
           && (cell_row(a) == cell_row(b) || cell_col(a) == cell_col(b)
               || cell_box(a) == cell_box(b));
}

bool cells_in_same_row(int cells[], int num_cells) {
    int row_first = cell_row(cells[0]);
    for (int i = 1; i < num_cells; i++) {
        if (cell_row(cells[i]) != row_first) return false;
    }
    return true;
}

bool cells_in_same_col(int cells[], int num_cells) {
    int col_first = cell_col(cells[0]);
    for (int i = 1; i < num_cells; i++) {
        if (cell_col(cells[i]) != col_first) return false;
    }
    return true;
}

bool cells_in_same_box(int cells[], int num_cells) {
    int box_first = cell_box(cells[0]);
    for (int i = 1; i < num_cells; i++) {
        if (cell_box(cells[i]) != box_first) return false;
    }
    return true;
}

int cells_common_peers(int cells[], int num_cells, int out[]) {
    int count = 0;
    for (int i = 0; i < NUM_PEERS; i++) {
        bool is_common = true;
        int possible_peer = peers[cells[0]][i];
        for (int j = 1; j < num_cells; j++) {
            if (!cells_are_peers(possible_peer, cells[j])) {
                is_common = false;
                break;
            }
        }
        if (is_common) {
            out[count++] = possible_peer;
        }
    }
    return count;
}

UnitType other_line(UnitType unit) {
    return unit == UNIT_ROW ? UNIT_COL : UNIT_ROW;
}
