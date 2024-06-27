#pragma once
#include <stdbool.h>
void obs_log(int, const char*, ...);
char* ltoa(long);
char* getHomePath(void);
extern int MAXNUMPATHS;
bool checkRepoExists(char**, int, char*);
char* extractUnicode(const char*);
