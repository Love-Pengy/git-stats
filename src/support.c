#include "support.h"

#include <obs-module.h>

const char* PLUGIN_NAME = "git-stats";
const char* PLUGIN_VERSION = "0.0.0";

void obs_log(int log_level, const char* format, ...) {
    size_t length = 4 + strlen(PLUGIN_NAME) + strlen(format);

    char* template = malloc(length + 1);

    snprintf(template, length, "[%s] %s", PLUGIN_NAME, format);

    va_list(args);

    va_start(args, format);
    blogva(log_level, template, args);
    va_end(args);

    free(template);
}
