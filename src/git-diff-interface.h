#pragma once
#include "./git-stats-source.h"
void updateTrackedFiles(struct gitData *);
void expandHomeDir(char **);
void createUntrackedFilesHM(struct gitData *);
void addGitRepoDir(struct gitData *, char *);
char *checkInvalidRepos(char **, int);
