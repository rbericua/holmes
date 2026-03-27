#include "util/dynstr.h"

#include <stdarg.h>
#include <stdio.h>

#include "util/dynarr.h"

void ds_init(DynStr *ds) {
    da_init(ds);
}

void ds_append(DynStr *ds, char *format, ...) {
    va_list args1, args2;
    va_start(args1, format);
    va_start(args2, format);

    int new_len = ds->len + vsnprintf(NULL, 0, format, args1);
    da_reserve(ds, new_len + 1);
    vsprintf(&ds->elems[ds->len], format, args2);
    ds->len = new_len;

    va_end(args1);
    va_end(args2);
}

void ds_clear(DynStr *ds) {
    da_clear(ds);
}

void ds_deinit(DynStr *ds) {
    da_deinit(ds);
}
