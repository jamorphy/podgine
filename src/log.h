#pragma once

#include <stdio.h>
#include <time.h>

extern FILE* g_log_file;

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

void init_logging(void);
void shutdown_logging(void);

#define LOG_LEVEL_CUTOFF LOG_DEBUG

#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RESET   "\x1b[0m"

#define LOG(level, fmt, ...) do { \
    if (level >= LOG_LEVEL_CUTOFF) { \
        char timestamp[20]; \
        time_t now; \
        struct tm* tm_info; \
        time(&now); \
        tm_info = localtime(&now); \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
        const char* level_str = \
            level == LOG_DEBUG ? "DEBUG" : \
            level == LOG_INFO ? "INFO" : \
            level == LOG_WARNING ? "WARN" : \
            level == LOG_ERROR ? "ERROR" : "FATAL"; \
        const char* color = \
            level == LOG_DEBUG ? "" : \
            level == LOG_INFO ? "" : \
            level == LOG_WARNING ? ANSI_YELLOW : \
            level == LOG_ERROR ? ANSI_RED : ANSI_RED; \
        fprintf(stderr, "[%s] [%s%s%s] " fmt "\n", \
                timestamp, color, level_str, ANSI_RESET, ##__VA_ARGS__); \
        if (g_log_file) { \
            fprintf(g_log_file, "[%s] [%s] " fmt "\n", \
                    timestamp, level_str, ##__VA_ARGS__); \
            fflush(g_log_file); \
        } \
        if (level == LOG_FATAL) { \
            fflush(stderr); \
            exit(1); \
        } \
    } \
} while(0)

#define LOG_DEBUG(fmt, ...)   LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)    LOG(LOG_WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   LOG(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)   LOG(LOG_FATAL, fmt, ##__VA_ARGS__)
