#include "bruteforce/dlx.h"

#include <stdlib.h>

#include "cand_set.h"
#include "geometry.h"
#include "grid.h"

#define NUM_COLUMNS (81 * 4)

struct Node;
typedef struct Node Node;

typedef struct {
    Node *head;
    int size;
} Column;

struct Node {
    Node *left;
    Node *right;
    Node *up;
    Node *down;
    Column *col;
};

static void create_columns(Column out[]);
static void create_row(Column cols[], int col_idxs[], int num_col_idxs);
static void fill_matrix(Grid *grid, Column cols[]);
static void free_matrix(Column cols[]);
static void search(Column *root, int *num_solutions);

static Column *find_best_column(Column *root);
static void cover_column(Column *col);
static void uncover_column(Column *col);

int solve_dlx(Grid *grid) {
    Column cols[NUM_COLUMNS + 1];
    create_columns(cols);
    Column *root = &cols[0];

    fill_matrix(grid, cols);

    int num_solutions = 0;
    search(root, &num_solutions);

    free_matrix(cols);
    return num_solutions;
}

static void create_columns(Column out[]) {
    Node *nodes = malloc((NUM_COLUMNS + 1) * sizeof(Node));

    Column *prev = &out[0];
    prev->head = &nodes[0];
    for (int i = 0; i < NUM_COLUMNS + 1; i++) {
        Node *head = &nodes[i];

        head->left = prev->head;
        prev->head->right = head;

        head->right = out[0].head;
        out[0].head->left = head;

        head->up = head->down = head;

        head->col = &out[i];

        out[i].head = head;
        out[i].size = 0;

        prev = &out[i];
    }
}

static void create_row(Column cols[], int col_idxs[], int num_col_idxs) {
    Node *nodes = malloc(num_col_idxs * sizeof(Node));
    Node *prev = &nodes[0];
    for (int i = 0; i < num_col_idxs; i++) {
        Node *node = &nodes[i];
        Column *col = &cols[col_idxs[i] + 1];
        col->size++;

        node->left = prev;
        prev->right = node;

        node->right = &nodes[0];
        nodes[0].left = node;

        node->up = col->head->up;
        col->head->up->down = node;

        node->down = col->head;
        col->head->up = node;

        node->col = col;

        prev = node;
    }
}

static void fill_matrix(Grid *grid, Column cols[]) {
    for (int i = 0; i < 81; i++) {
        CandSet cands = grid_cell_is_empty(grid, i)
                            ? grid_cell_cands(grid, i)
                            : cand_set_from_value(grid_cell_value(grid, i));
        while (!cand_set_is_empty(cands)) {
            int cand_idx = cand_set_pop(&cands) - 1;
            int col_idxs[] = {
                i,
                81 + cell_row(i) * 9 + cand_idx,
                81 * 2 + cell_col(i) * 9 + cand_idx,
                81 * 3 + cell_box(i) * 9 + cand_idx,
            };
            create_row(cols, col_idxs, 4);
        }
    }
}

static void free_matrix(Column cols[]) {
    for (int i = 1; i <= 81; i++) {
        Node *curr = cols[i].head->down;
        while (curr != cols[i].head) {
            Node *next = curr->down;
            free(curr);
            curr = next;
        }
    }
    free(cols[0].head);
}

static Column *find_best_column(Column *root) {
    Node *curr = root->head->right;
    Column *best_col = curr->col;
    while (curr != root->head) {
        if (curr->col->size < best_col->size) {
            best_col = curr->col;
        }
        curr = curr->right;
    }
    return best_col;
}

static void cover_column(Column *col) {
    col->head->left->right = col->head->right;
    col->head->right->left = col->head->left;

    for (Node *i = col->head->down; i != col->head; i = i->down) {
        for (Node *j = i->right; j != i; j = j->right) {
            j->up->down = j->down;
            j->down->up = j->up;
            j->col->size--;
        }
    }
}

static void uncover_column(Column *col) {
    for (Node *i = col->head->up; i != col->head; i = i->up) {
        for (Node *j = i->left; j != i; j = j->left) {
            j->up->down = j;
            j->down->up = j;
            j->col->size++;
        }
    }

    col->head->left->right = col->head;
    col->head->right->left = col->head;
}

static void search(Column *root, int *num_solutions) {
    if (root->head == root->head->right) {
        (*num_solutions)++;
        return;
    }

    Column *col = find_best_column(root);
    if (col->size == 0) return;

    cover_column(col);
    for (Node *i = col->head->down; i != col->head; i = i->down) {
        for (Node *j = i->right; j != i; j = j->right) {
            cover_column(j->col);
        }
        search(root, num_solutions);
        for (Node *j = i->left; j != i; j = j->left) {
            uncover_column(j->col);
        }
        if (*num_solutions > 1) break;
    }
    uncover_column(col);
}
