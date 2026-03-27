#include "util/log.h"

#include <stdarg.h>
#include <stdio.h>

#define LOG_FILE "holmes.log"

FILE *log_file;
LogLevel min_level = LOG_INFO;
char *level_strs[] = {
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO] = "INFO",
    [LOG_WARN] = "WARN",
    [LOG_ERROR] = "ERROR",
};

void log_init(void) {
    log_file = fopen(LOG_FILE, "a");
}

void log_deinit(void) {
    fclose(log_file);
}

void log_set_level(LogLevel level) {
    min_level = level;
}

void log_log(LogLevel level, char *file, int line, char *format, ...) {
    if (level < min_level) return;

    va_list args;
    va_start(args, format);

    fprintf(log_file, "[%s] %s:%d: ", level_strs[level], file, line);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    fflush(log_file);

    va_end(args);
}
