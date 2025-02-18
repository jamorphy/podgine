#include <stdlib.h>
#include <unistd.h>

#include "log.h"

FILE* g_log_file = NULL;

void init_logging(void)
{
    g_log_file = fopen("engine.log", "w");
    if (!g_log_file) {
        perror("Failed to open log file");
        exit(1);
    }
}

void shutdown_logging(void)
{
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}
