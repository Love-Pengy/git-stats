#pragma once

typedef struct fileType* untrackedFile;

char* getUntrackedFilePath(untrackedFile);
struct tm* getModifiedTime(char*);
void updateUntrackedFile(untrackedFile*);
long getLinesAdded(untrackedFile);
untrackedFile createUntrackedFile(char*);
char* untrackedFileToString(untrackedFile);
int getIdNum(untrackedFile);
void freeUntrackedFile(untrackedFile*);
