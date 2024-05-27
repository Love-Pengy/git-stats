#pragma once
#include "hashTable.h"
typedef struct fileType* untrackedFile;
void freeUntrackedFile(untrackedFile*);
struct tm* getModifiedTime(char*);
long getLinesInFile(char*);
struct tm getTimeEdited(untrackedFile);
long getLinesAdded(untrackedFile);
untrackedFile createUntrackedFile(char*);
char* untrackedFileToString(untrackedFile);
void copyUntrackedFile(untrackedFile*, untrackedFile*);
void updateUntrackedFile(untrackedFile*);
