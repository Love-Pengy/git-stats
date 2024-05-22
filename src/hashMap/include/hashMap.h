#pragma once

#include "../lib/include/hashTable.h"
#include "../lib/include/untrackedFile.h"

typedef struct hashMapType* hashMap;

hashMap createHashMap(void);
void removeElementHM(hashMap*, char*);
void addElementHM(hashMap*, char*, untrackedFile);
untrackedFile findValueHM(hashMap, char*);
void printHM(hashMap);
void freeHM(hashMap*);
void updateValueHM(hashMap*);
long getLinesAddedHM(hashMap*);
