#pragma once
#include "../include/boardingPassExample.h"
typedef struct hashType* hashTable;

void freeHashTable(hashTable*);
hashTable createHT(void);
void addElementHT(hashTable*, char*, boardingPass);
void removeElementHT(hashTable*, char*);
boardingPass findValueHT(hashTable, char*);
void printHT(hashTable);
hashTable rehashHashTable(hashTable*);
