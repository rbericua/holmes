#include "dynstr.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dynarr.h"

void ds_init(DynStr *ds) {
    da_init(ds);
}

void ds_append(DynStr *ds, char *str) {
    int len = strlen(str);
    da_reserve(ds, ds->len + len);
    strncpy(ds->elems + ds->len, str, len);
    ds->len += len;
}

void ds_appendf(DynStr *ds, char *format, ...) {
    va_list args;
    va_start(args, format);

    ds_vappendf(ds, format, args);

    va_end(args);
}

void ds_vappendf(DynStr *ds, char *format, va_list args) {
    va_list args1, args2;
    va_copy(args1, args);
    va_copy(args2, args);

    int len = vsnprintf(NULL, 0, format, args1);
    da_reserve(ds, ds->len + len + 1);
    vsnprintf(ds->elems + ds->len, len + 1, format, args2);

    va_end(args1);
    va_end(args2);

    ds->len += len;
}

void ds_append_null(DynStr *ds) {
    ds_append(ds, "\0");
}

void ds_clear(DynStr *ds) {
    da_clear(ds);
}

void ds_deinit(DynStr *ds) {
    da_deinit(ds);
}
