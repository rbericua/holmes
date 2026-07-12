#include "ui/colors.h"

#include <ncurses.h>

int color_attr(ColorPair color) {
    int attr = COLOR_PAIR(color);
    if (color >= CP_REVERSE_START) {
        attr |= A_REVERSE;
    }
    return attr;
}
