#include "ui/pipes.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "ui/layout.h"
#include "util/dynarr.h"

#define GRIDLINE_PENALTY 1000
#define OVERLAP_PENALTY 100
#define TURN_PENALTY 10

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SIGN(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)

typedef enum {
    ORIENT_VERTICAL,
    ORIENT_HORIZONTAL,

    NUM_ORIENTATIONS
} Orientation;

typedef enum {
    NODE_UNVISITED,
    NODE_OPEN,
    NODE_CLOSED
} NodeStatus;

struct Node {
    Position pos;
    int f, g, h;
    Orientation orient;
    NodeStatus status;
    struct Node *prev;

    int heap_idx;
    int heap_insertion;
};
typedef struct Node Node;

typedef struct {
    bool orients[GRID_HEIGHT][GRID_WIDTH][NUM_ORIENTATIONS];
    Node nodes[GRID_HEIGHT][GRID_WIDTH][NUM_ORIENTATIONS];
} RoutingMap;

#define HEAP_INIT_CAP 16
#define HEAP_GROWTH_FACTOR 2

#define HEAP_PARENT(i) (((i) - 1) / 2)
#define HEAP_LEFT_CHILD(i) (2 * (i) + 1)
#define HEAP_RIGHT_CHILD(i) (2 * (i) + 2)

typedef struct {
    Node **nodes;
    int len;
    int cap;
    int counter;
} Heap;

static void route_pipe(Pipe *pipe, RoutingMap *map);
static int calculate_cost(Position prev, Position curr, Position next,
                          RoutingMap *map);
static int calculate_heuristic(Position prev, Position curr, Position tgt);
static void reconstruct_path(Node *target, Positions *path, RoutingMap *map);

static void cell_cand_to_pos(int cell, int cand, Position *pos);
static void pipe_deltas(Pipe *pipe, int *delta_y, int *delta_x);
static int pipe_cmp(Pipe *pipe1, Pipe *pipe2);
static bool is_out_of_bounds(Position pos);
static int taxicab(Position pos1, Position pos2);
static Orientation get_orientation(Position from, Position to);
static Direction get_straight_dir(Position from, Position to);
static Direction get_corner_dir(Direction dir1, Direction dir2);

static int node_cmp(Node *node1, Node *node2);
static Heap *heap_create(void);
static void heap_destroy(Heap *h);
static bool heap_is_empty(Heap *h);
static Node *heap_extract_min(Heap *h);
static void heap_insert(Heap *h, Node *node);
static void heap_update(Heap *h, Node *node);
static void heap_swap_nodes(Heap *h, int i, int j);
static void heap_bubble_up(Heap *h, int from);
static void heap_bubble_down(Heap *h, int from);

Pipe pipe_create(int cell1, int cand1, int cell2, int cand2) {
    Pipe pipe;
    cell_cand_to_pos(cell1, cand1, &pipe.pos1);
    cell_cand_to_pos(cell2, cand2, &pipe.pos2);
    da_init(&pipe.path);
    return pipe;
}

void pipe_destroy(Pipe *pipe) {
    da_deinit(&pipe->path);
}

void route_pipes(Pipes *pipes) {
    qsort(pipes->elems, pipes->len, sizeof(Pipe),
          (int (*)(const void *, const void *))pipe_cmp);

    RoutingMap *map = malloc(sizeof(RoutingMap));

    for (int i = 1; i < GRID_HEIGHT - 1; i++) {
        for (int j = 1; j < GRID_WIDTH - 1; j++) {
            for (int orient = 0; orient < NUM_ORIENTATIONS; orient++) {
                map->nodes[i][j][orient].pos = (Position){i, j};
                map->nodes[i][j][orient].orient = orient;
                map->orients[i][j][orient] = false;
            }
        }
    }

    for (int i = 0; i < pipes->len; i++) {
        route_pipe(&pipes->elems[i], map);
    }

    free(map);
}

Direction get_direction(Position prev, Position curr, Position next) {
    Direction dir1 = get_straight_dir(prev, curr);
    Direction dir2 = get_straight_dir(curr, next);
    if (dir1 == dir2) return dir1;
    return get_corner_dir(dir1, dir2);
}

static void route_pipe(Pipe *pipe, RoutingMap *map) {
    Position src = pipe->pos1;
    Position tgt = pipe->pos2;

    for (int i = 1; i < GRID_HEIGHT - 1; i++) {
        for (int j = 1; j < GRID_WIDTH - 1; j++) {
            for (int orient = 0; orient < NUM_ORIENTATIONS; orient++) {
                map->nodes[i][j][orient].g = INT_MAX;
                map->nodes[i][j][orient].f = INT_MAX;
                map->nodes[i][j][orient].status = NODE_UNVISITED;
                map->nodes[i][j][orient].prev = NULL;
            }
        }
    }

    Node *src_node = &map->nodes[src.y][src.x][ORIENT_VERTICAL];
    src_node->g = 0;
    src_node->h = calculate_heuristic((Position){-1, -1}, src, tgt);
    src_node->f = src_node->h;

    Heap *open_nodes = heap_create();
    heap_insert(open_nodes, src_node);
    map->nodes[src.y][src.x][ORIENT_HORIZONTAL].status = NODE_CLOSED;

    int dy[] = {-1, 0, 0, 1};
    int dx[] = {0, -1, 1, 0};
    Orientation dor[] = {ORIENT_VERTICAL, ORIENT_HORIZONTAL, ORIENT_HORIZONTAL,
                         ORIENT_VERTICAL};

    while (!heap_is_empty(open_nodes)) {
        Node *curr = heap_extract_min(open_nodes);
        curr->status = NODE_CLOSED;

        Position prev_pos = curr->prev ? curr->prev->pos : (Position){-1, -1};

        if (curr->pos.y == tgt.y && curr->pos.x == tgt.x) {
            reconstruct_path(curr, &pipe->path, map);
            heap_destroy(open_nodes);
            return;
        }

        for (int i = 0; i < 4; i++) {
            int ny = curr->pos.y + dy[i];
            int nx = curr->pos.x + dx[i];
            if (is_out_of_bounds((Position){ny, nx})
                || (prev_pos.y == ny && prev_pos.x == nx))
                continue;

            Node *next = &map->nodes[ny][nx][dor[i]];
            if (next->status == NODE_CLOSED) continue;

            int tentative_cost = curr->g
                                 + calculate_cost(prev_pos, curr->pos,
                                                  next->pos, map);
            if (tentative_cost < next->g) {
                next->g = tentative_cost;
                next->h = calculate_heuristic(curr->pos, next->pos, tgt);
                next->f = next->g + next->h;
                next->prev = curr;

                if (next->status == NODE_UNVISITED) {
                    next->status = NODE_OPEN;
                    heap_insert(open_nodes, next);
                } else {
                    heap_update(open_nodes, next);
                }
            }
        }
    }

    heap_destroy(open_nodes);
}

static int calculate_cost(Position prev, Position curr, Position next,
                          RoutingMap *map) {
    int cost = 1;

    Orientation next_orient = get_orientation(curr, next);
    Orientation prev_orient = (prev.y != -1 && prev.x != -1)
                                  ? get_orientation(prev, curr)
                                  : next_orient;

    if ((next_orient == ORIENT_HORIZONTAL && next.y % (CELL_HEIGHT + 1) == 0)
        || (next_orient == ORIENT_VERTICAL && next.x % (CELL_WIDTH + 1) == 0)) {
        cost += GRIDLINE_PENALTY;
    }

    if (map->orients[curr.y][curr.x][next_orient]
        || map->orients[next.y][next.x][next_orient]) {
        cost += OVERLAP_PENALTY;
    }

    if (prev_orient != next_orient) {
        cost += TURN_PENALTY;
    }

    return cost;
}

static int turns_needed(Position prev, Position curr, Position tgt) {
    Orientation needed_orient = get_orientation(curr, tgt);
    Orientation prev_orient = (prev.y != -1 && prev.x != -1)
                                  ? get_orientation(prev, curr)
                                  : needed_orient;

    if ((curr.y != tgt.y && curr.x == tgt.x)
        || (curr.y == tgt.y && curr.x != tgt.x)) {
        return prev_orient != needed_orient;
    }
    return curr.y != tgt.y || curr.x != tgt.x;
}

static int calculate_heuristic(Position prev, Position curr, Position tgt) {
    return taxicab(curr, tgt) + turns_needed(prev, curr, tgt) * TURN_PENALTY;
}

static void reconstruct_path(Node *target, Positions *path, RoutingMap *map) {
    bool first = true;
    Node *curr = target;
    while (curr) {
        if (!first && curr->prev) {
            Orientation orient = get_orientation(curr->prev->pos, curr->pos);
            map->orients[curr->pos.y][curr->pos.x][orient] = true;
        }

        da_append(path, curr->pos);
        curr = curr->prev;
        first = false;
    }
}

static void cell_cand_to_pos(int cell, int cand, Position *pos) {
    pos->y = cell_row(cell) * (CELL_HEIGHT + 1) + (cand - 1) / 3 + 1;
    pos->x = cell_col(cell) * (CELL_WIDTH + 1) + ((cand - 1) % 3) * 3 + 2;
}

static void pipe_deltas(Pipe *pipe, int *delta_y, int *delta_x) {
    *delta_y = ABS(pipe->pos1.y - pipe->pos2.y);
    *delta_x = ABS(pipe->pos1.x - pipe->pos2.x);
}

static int pipe_cmp(Pipe *pipe1, Pipe *pipe2) {
    int delta_y1, delta_x1;
    int delta_y2, delta_x2;

    pipe_deltas(pipe1, &delta_y1, &delta_x1);
    pipe_deltas(pipe2, &delta_y2, &delta_x2);

    bool is_straight1 = delta_y1 == 0 || delta_x1 == 0;
    bool is_straight2 = delta_y2 == 0 || delta_x2 == 0;

    int len1 = delta_y1 + delta_x1;
    int len2 = delta_y2 + delta_x2;

    if (is_straight1 && !is_straight2) return -1;
    if (!is_straight1 && is_straight2) return 1;

    return len1 - len2;
}

static bool is_out_of_bounds(Position pos) {
    return (pos.y <= 0 || pos.y >= GRID_HEIGHT - 1)
           || (pos.x <= 0 || pos.x >= GRID_WIDTH - 1);
}

static int taxicab(Position pos1, Position pos2) {
    return ABS(pos1.y - pos2.y) + ABS(pos1.x - pos2.x);
}

static Orientation get_orientation(Position from, Position to) {
    return from.y == to.y ? ORIENT_HORIZONTAL : ORIENT_VERTICAL;
}

static Direction get_straight_dir(Position from, Position to) {
    int dy = SIGN(to.y - from.y);
    int dx = SIGN(to.x - from.x);

    return dy == 1    ? DIR_DOWN
           : dy == -1 ? DIR_UP
           : dx == 1  ? DIR_RIGHT
                      : DIR_LEFT;
}

static Direction get_corner_dir(Direction dir1, Direction dir2) {
    if ((dir1 == DIR_DOWN && dir2 == DIR_RIGHT)
        || (dir1 == DIR_LEFT && dir2 == DIR_UP))
        return DIR_DOWN_RIGHT;

    if ((dir1 == DIR_DOWN && dir2 == DIR_LEFT)
        || (dir1 == DIR_RIGHT && dir2 == DIR_UP))
        return DIR_DOWN_LEFT;

    if ((dir1 == DIR_UP && dir2 == DIR_RIGHT)
        || (dir1 == DIR_LEFT && dir2 == DIR_DOWN))
        return DIR_UP_RIGHT;

    return DIR_UP_LEFT;
}

static int node_cmp(Node *node1, Node *node2) {
    if (node1->f != node2->f) return node1->f - node2->f;
    return node2->heap_insertion - node1->heap_insertion;
}

static Heap *heap_create(void) {
    Heap *h = malloc(sizeof(Heap));
    h->nodes = malloc(HEAP_INIT_CAP * sizeof(Node *));
    h->len = 0;
    h->cap = HEAP_INIT_CAP;
    h->counter = 0;
    return h;
}

static void heap_destroy(Heap *h) {
    free(h->nodes);
    free(h);
}

static bool heap_is_empty(Heap *h) {
    return h->len == 0;
}

static Node *heap_extract_min(Heap *h) {
    Node *ret = h->nodes[0];

    h->nodes[0] = h->nodes[--h->len];
    h->nodes[0]->heap_idx = 0;
    heap_bubble_down(h, 0);

    return ret;
}

static void heap_insert(Heap *h, Node *node) {
    if (h->len >= h->cap) {
        h->cap *= HEAP_GROWTH_FACTOR;
        h->nodes = realloc(h->nodes, h->cap * sizeof(Node *));
    }

    node->heap_insertion = h->counter++;
    h->nodes[h->len] = node;
    h->nodes[h->len]->heap_idx = h->len;
    heap_bubble_up(h, h->len);
    h->len++;
}

static void heap_update(Heap *h, Node *node) {
    node->heap_insertion = h->counter++;
    heap_bubble_up(h, node->heap_idx);
    heap_bubble_down(h, node->heap_idx);
}

static void heap_swap_nodes(Heap *h, int i, int j) {
    Node *temp = h->nodes[i];
    h->nodes[i] = h->nodes[j];
    h->nodes[j] = temp;
    h->nodes[i]->heap_idx = i;
    h->nodes[j]->heap_idx = j;
}

static void heap_bubble_up(Heap *h, int from) {
    if (from == 0) return;

    int parent = HEAP_PARENT(from);

    if (node_cmp(h->nodes[parent], h->nodes[from]) < 0) return;

    heap_swap_nodes(h, from, parent);

    heap_bubble_up(h, parent);
}

static void heap_bubble_down(Heap *h, int from) {
    int left = HEAP_LEFT_CHILD(from);
    int right = HEAP_RIGHT_CHILD(from);

    if (left >= h->len) return;

    int min = right < h->len && node_cmp(h->nodes[right], h->nodes[left]) < 0
                  ? right
                  : left;

    if (node_cmp(h->nodes[from], h->nodes[min]) < 0) return;

    heap_swap_nodes(h, from, min);

    heap_bubble_down(h, min);
}
