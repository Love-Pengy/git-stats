#include "../include/hashTable.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/boardingPassExample.h"

#define CONST_MULTIPLIER 287987
int initialSize = 3;

// example using border passes
struct hashType {
    boardingPass* passes;
    char** keys;
    int size;
};

void freeHashTable(hashTable* table) {
    for (int i = 0; i < (*table)->size; i++) {
        free((*table)->passes[i]);
        free((*table)->keys[i]);
    }
    free((*table));
    (*table) = NULL;
}

hashTable rehashHashTable(hashTable* table) {
    int oldMax = (*table)->size;
    int newSize = oldMax * 2;
    hashTable meow = malloc(sizeof(struct hashType));
    meow->passes = malloc(sizeof(boardingPass) * newSize);
    meow->keys = malloc(sizeof(char*) * newSize);
    meow->size = newSize;
    for (int i = 0; i < newSize; i++) {
        meow->passes[i] = NULL;
        meow->keys[i] = NULL;
    }
    for (int i = 0; i < oldMax; i++) {
        addElementHT(&meow, (*table)->keys[i], (*table)->passes[i]);
    }
    // freeHashTable(table);
    return (meow);
}

bool keyExists(hashTable table, char* key) {
    if (table == NULL) {
        return (false);
    }
    bool found = false;
    for (int i = 0; i < table->size; i++) {
        if (!(table->keys[i] == NULL) && !strcmp(key, table->keys[i])) {
            found = true;
            break;
        }
    }
    if (found) {
        return (true);
    }
    return (false);
}

hashTable createHT(void) {
    hashTable meow = malloc(sizeof(struct hashType));
    meow->passes = malloc(sizeof(boardingPass) * initialSize);
    meow->keys = malloc(sizeof(char*) * initialSize);
    for (int i = 0; i < initialSize; i++) {
        meow->passes[i] = NULL;
        meow->keys = malloc(sizeof(NULL) + 1);
        meow->keys[i] = NULL;
    }
    meow->size = initialSize;
    return (meow);
}

int asciiSum(char* string) {
    int val = 0;
    for (int i = 0; i < strlen(string); i++) {
        val += (int)string[i];
    }
    return (val);
}

// hashes key and returns the index
int hashKey(char* key, int numCollisions, int tableSize) {
    // get to the end and then start at 0 with your current offsets
    int keyVal = asciiSum(key);
    int hash = (CONST_MULTIPLIER * keyVal) % tableSize;
    if ((hash + numCollisions) > tableSize) {
        return (numCollisions - (tableSize - hash));
    }
    return (hash + numCollisions);
}

void addElementHT(hashTable* table, char* key, boardingPass value) {
    int numCollisions = 0;
    bool notFull = true;
    if ((*table) == NULL) {
    }
    else {
        while ((*table)->passes[hashKey(key, numCollisions, (*table)->size)] !=
               NULL) {
            if (numCollisions == (*table)->size) {
                notFull = false;
                break;
            }
            numCollisions++;
        }
        if (notFull) {
            (*table)->passes[hashKey(key, numCollisions, (*table)->size)] =
                malloc(sizeof(struct hashType));
            (*table)->passes[hashKey(key, numCollisions, (*table)->size)] =
                value;

            (*table)->keys[hashKey(key, numCollisions, (*table)->size)] =
                malloc(sizeof(char) * strlen(key) + 1);
            strcpy(
                (*table)->keys[hashKey(key, numCollisions, (*table)->size)],
                key);
        }
    }
}
void removeElementHT(hashTable* table, char* key) {
    int numCollisions = 0;

    if (((*table) != NULL) && keyExists((*table), key)) {
        while (1) {
            if ((!((*table)->keys[hashKey(
                       key, numCollisions, (*table)->size)] == NULL)) &&
                !strcmp(
                    (*table)->keys[hashKey(key, numCollisions, (*table)->size)],
                    key)) {
                break;
            }
            numCollisions++;
        }
        free((*table)->keys[hashKey(key, numCollisions, (*table)->size)]);
        free((*table)->passes[hashKey(key, numCollisions, (*table)->size)]);
        (*table)->passes[hashKey(key, numCollisions, (*table)->size)] =
            malloc(sizeof(struct hashType));
        (*table)->passes[hashKey(key, numCollisions, (*table)->size)] = NULL;

        (*table)->keys[hashKey(key, numCollisions, (*table)->size)] =
            malloc(sizeof(struct hashType));
        (*table)->keys[hashKey(key, numCollisions, (*table)->size)] = NULL;
    }
}

boardingPass findValueHT(hashTable table, char* key) {
    int numCollisions = 0;
    bool found = false;
    if (table == NULL) {
        return (NULL);
    }
    while (table->passes[hashKey(key, numCollisions, table->size)] != NULL) {
        if ((!strcmp(
                key, table->keys[hashKey(key, numCollisions, table->size)]))) {
            found = true;
            break;
        }
        else if (
            (table->keys[numCollisions] != NULL) &&
            (numCollisions == (table->size - 1))) {
            break;
        }
        numCollisions++;
    }
    if (found) {
        return (table->passes[hashKey(key, numCollisions, table->size)]);
    }
    return (NULL);
}

void printHT(hashTable table) {
    if (table == NULL) {
        printf("{ EMPTY }\n");
    }
    else {
        for (int i = 0; i < table->size; i++) {
            if (i == 0) {
                printf(
                    "{ %s: %s, ", table->keys[i],
                    boardingPassToString(table->passes[i]));
            }

            else if (i == (table->size - 1)) {
                printf(
                    "%s: %s }\n", table->keys[i],
                    boardingPassToString(table->passes[i]));
            }

            else {
                printf(
                    "%s: %s, ", table->keys[i],
                    boardingPassToString(table->passes[i]));
            }
        }
    }
}
