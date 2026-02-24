#ifndef DYNSTR_H
#define DYNSTR_H

#include <stdarg.h>

typedef struct {
    char *elems;
    int len;
    int cap;
} DynStr;

void ds_init(DynStr *ds);
void ds_append(DynStr *ds, char *str);
void ds_appendf(DynStr *ds, char *format, ...)
    __attribute__((format(printf, 2, 3)));
void ds_vappendf(DynStr *ds, char *format, va_list args);
void ds_append_null(DynStr *ds);
void ds_clear(DynStr *ds);
void ds_deinit(DynStr *ds);

#endif
