#pragma once
#include "./git-stats-source.h"
void updateTrackedFiles(struct gitData *);
void expandHomeDir(char **);
void createUntrackedFiles(struct gitData *);
long updateUntrackedFiles(struct gitData *);
void addGitRepoDir(struct gitData *, char *);
bool checkUntrackedFiles(struct gitData *);
char *checkInvalidRepos(char **, int);
bool checkPath(char *);
