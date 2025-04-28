#pragma once

#include <stdbool.h>

char *extract_unicode(const char *);
void pad_string_left(char*, int);
void pad_string_right(char*, int);

// for useful information in debugging
#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
// for information to be displayed in log
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
// for problems that are recoverable 
#define warning(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)
// for problems that can affect the program, but don't require termination
#define error(format, ...) blog(LOG_ERROR, format, ##__VA_ARGS__)
