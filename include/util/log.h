#ifndef LOG_H
#define LOG_H

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_NONE
} LogLevel;

#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

void log_init(void);
void log_deinit(void);
void log_set_level(LogLevel level);
void log_log(LogLevel level, char *file, int line, char *format, ...)
    __attribute__((format(printf, 4, 5)));

#endif
