#pragma once

typedef struct fileType* untrackedFile;

untrackedFile createUntrackedFile(int, int, struct tm);
char* untrackedFileToString(untrackedFile);
int getIdNum(untrackedFile);
int getLinesAdded(untrackedFile);
