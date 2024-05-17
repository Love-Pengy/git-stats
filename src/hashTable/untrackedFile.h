#pragma once

typedef struct fileType* untrackedFile;

struct tm getTimeEdited(untrackedFile);
long getLinesAdded(untrackedFile);
untrackedFile createUntrackedFile(char*);
char* untrackedFileToString(untrackedFile);
int getIdNum(untrackedFile);
