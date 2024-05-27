#include "support.h"

#include <obs-module.h>
#include <stdlib.h>
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

char* ltoa(long input) {
    // get a given value by dividing by a multiple of 10
    //  check if it's the end by first getting the lenght by seeing if it
    //  divides by a multiple without being a fraction
    long currProduct = 1;
    int index = 0;
    char* output = malloc(sizeof(char) * 22);
    output[0] = '\0';
    if (input == 0) {
        output[0] = '0';
        output[1] = '\0';
    }
    while (!((input / currProduct) < 1)) {
        // add 48 because of ascii table
        output[index] = (char)(((input / currProduct) % 10) + 48);
        output[index + 1] = '\0';
        currProduct *= 10;
        index++;
    }
    printf("%s\n", output);
    return (output);
}

char* getHomePath(void) {
    if (getenv("HOME") == NULL) {
        return (NULL);
    }
    return (getenv("HOME"));
}
