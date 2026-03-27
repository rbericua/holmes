#include "geometry.h"

int units[3][81][81];
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
