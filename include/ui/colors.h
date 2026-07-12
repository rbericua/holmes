#ifndef COLORS_H
#define COLORS_H

typedef enum {
    CP_NORMAL,
    CP_CLUE,
    CP_LINK = CP_CLUE,

    CP_REVERSE_START,

    CP_TRIGGER = CP_REVERSE_START,
    CP_REMOVAL,
    CP_SPECIAL1,
    CP_SPECIAL2
} ColorPair;

int color_attr(ColorPair color);

#endif
