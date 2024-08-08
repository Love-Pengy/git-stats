#pragma once
#include <stdbool.h>
#include "git-stats-source.h"
void obs_log(int, const char *, ...);
char *ltoa(long);
char *getHomePath(void);
extern int MAXNUMPATHS;
bool checkRepoExists(char **, int, char *);
char *extractUnicode(const char *);
bool checkLockStatus(char *);
bool checkUntrackedFileLock(struct gitData *);
