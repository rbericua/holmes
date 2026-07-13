#ifndef PIPES_H
#define PIPES_H

#include <stdbool.h>

typedef enum {
    DIR_DOWN,
    DIR_UP,
    DIR_RIGHT,
    DIR_LEFT,
    DIR_DOWN_RIGHT,
    DIR_DOWN_LEFT,
    DIR_UP_RIGHT,
    DIR_UP_LEFT
} Direction;

typedef struct {
    int y, x;
} Position;

typedef struct {
    Position *elems;
    int len;
    int cap;
} Positions;

typedef struct {
    Position pos1, pos2;
    Positions path;
} Pipe;

typedef struct {
    Pipe *elems;
    int len;
    int cap;
} Pipes;

Pipe pipe_create(int cell1, int cand1, int cell2, int cand2);
void pipe_destroy(Pipe *pipe);
void route_pipes(Pipes *pipes);
Direction get_direction(Position prev, Position curr, Position next);

#endif
