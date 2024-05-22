#pragma once
#include "untrackedFile.h"

typedef struct hashType* hashTable;
typedef struct fileType* untrackedFile;
void freeHashTable(hashTable*);
hashTable createHT(void);
void addElementHT(hashTable*, char*, untrackedFile);
void removeElementHT(hashTable*, char*);
untrackedFile findValueHT(hashTable, char*);
void printHT(hashTable);
void rehashHashTable(hashTable*);
void updateValueHT(hashTable*, char*, untrackedFile);
