#include "bruteforce/dlx.h"

#include <limits.h>
#include <stdlib.h>

#include "grid.h"
#include "bruteforce/patterns.h"
#include "util/bits.h"

#define NUM_COLUMNS (9 + 81)

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

static void fill_value_patterns(Grid *grid, Bitset128 out[]);
static void create_columns(Column out[]);
static void create_row(Column cols[], int value, Bitset128 pattern);
static void fill_matrix(Bitset128 value_patterns[], Column cols[]);
static void free_matrix(Column cols[]);
static void search(Column *root, int *num_solutions);

static Column *find_best_column(Column *root);
static void cover_column(Column *col);
static void uncover_column(Column *col);

int solve_dlx(Grid *grid) {
    Bitset128 value_patterns[9];
    fill_value_patterns(grid, value_patterns);

    Column cols[NUM_COLUMNS + 1];
    create_columns(cols);
    Column *root = &cols[0];

    fill_matrix(value_patterns, cols);

    int num_solutions = 0;
    search(root, &num_solutions);

    free_matrix(cols);
    // free(cols[0].head);
    return num_solutions;
}

static void fill_value_patterns(Grid *grid, Bitset128 out[]) {
    for (int i = 0; i < 9; i++) {
        out[i] = BITSET128_EMPTY;
    }
    for (int i = 0; i < 81; i++) {
        if (grid_cell_is_empty(grid, i)) continue;
        int value = grid_cell_value(grid, i);
        out[value - 1] = bitset128_add(out[value - 1], i);
    }
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

static void create_row(Column cols[], int value, Bitset128 pattern) {
    int node_idxs[10] = {[0] = value};
    int j = 1;
    while (!bitset128_is_empty(pattern)) {
        node_idxs[j++] = bitset128_pop(&pattern) + 10;
    }

    Node *nodes = malloc(10 * sizeof(Node));
    Node *prev = &nodes[0];
    for (int i = 0; i < 10; i++) {
        Node *node = &nodes[i];
        Column *col = &cols[node_idxs[i]];
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

static void fill_matrix(Bitset128 value_patterns[], Column cols[]) {
    Bitset128 *patterns = generate_patterns();
    for (int value = 1; value <= 9; value++) {
        for (int pattern_i = 0; pattern_i < NUM_PATTERNS; pattern_i++) {
            Bitset128 pattern = patterns[pattern_i];
            if (bitset128_is_sub(value_patterns[value - 1], pattern)) {
                create_row(cols, value, pattern);
            }
        }
    }
    free_patterns(patterns);
}

static void free_matrix(Column cols[]) {
    for (int i = 1; i < 10; i++) {
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
