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
    int size = 0;
    char* output = malloc(sizeof(char) * 22);
    bool entered = false;
    output[0] = '\0';
    if (input == 0) {
        output[0] = '0';
        output[1] = '\0';
    }
    while (!((input / currProduct) < 1)) {
        if (currProduct != 1) {
            entered = true;
        }
        size++;
        currProduct *= 10;
    }
    currProduct = 1;
    for (int i = 0; i < size; i++) {
        // 0th index is highest one
        // add 48 because of ascii table
        output[i] =
            (char)((((int)(input / powl(10, ((size - 1) - i))) % 10) + 48));
        currProduct *= 10;
    }
    output[size] = '\0';
    if (!entered) {
        output[0] = (char)(input + 48);
        output[1] = '\0';
    }
    return (output);
}

char* getHomePath(void) {
    if (getenv("HOME") == NULL) {
        return (NULL);
    }
    return (getenv("HOME"));
}
