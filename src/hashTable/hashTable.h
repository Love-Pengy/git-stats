#pragma once
#include "untrackedFile.h"

typedef struct hashType* hashTable;
void freeHashTable(hashTable*);
hashTable createHT(void);
void addElementHT(hashTable*, char*, untrackedFile);
void removeElementHT(hashTable*, char*);
untrackedFile findValueHT(hashTable, char*);
void printHT(hashTable);
hashTable rehashHashTable(hashTable*);
