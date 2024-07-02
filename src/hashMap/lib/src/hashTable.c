#include "../include/hashTable.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/untrackedFile.h"

#define CONST_MULTIPLIER 287987
int initialSize = 10;

// example using border passes
struct hashType {
    untrackedFile* files;
    char** keys;
    int size;
};

long getLinesAddedHT(hashTable* table) {
    long added = 0;
    for (int i = 0; i < (*table)->size; i++) {
        if ((*table)->files[i] == NULL) {
            continue;
        }
        added += getLinesAdded((*table)->files[i]);
    }
    return (added);
}

void autoUpdateValuesHT(hashTable* table) {
    for (int i = 0; i < (*table)->size; i++) {
        if ((*table)->keys[i] == NULL) {
            continue;
        }
        else {
            updateUntrackedFile(&((*table)->files[i]));
        }
    }
}

void updateValueHT(hashTable* table, char* key, untrackedFile value) {
    for (int i = 0; i < (*table)->size; i++) {
        if ((*table)->keys[i] == NULL) {
            continue;
        }
        if (!strcmp((*table)->keys[i], key)) {
            (*table)->files[i] = value;
            break;
        }
    }
}

void freeHashTable(hashTable* table) {
    if (*table) {
        for (int i = 0; i < (*table)->size; i++) {
            freeUntrackedFile(&((*table)->files[i]));
            free((*table)->keys[i]);
            (*table)->keys[i] = NULL;
        }
        free((*table));
        (*table) = NULL;
    }
}

// deep copy value in hash table into dest
void copyHashTable(hashTable* dest, hashTable* src) {
    if ((*src) == NULL) {
        (*dest) = NULL;
        return;
    }
    (*dest)->size = (*src)->size;
    for (int i = 0; i < (*src)->size; i++) {
        // if pass is null key must be null
        if ((*src)->files[i] == NULL) {
            continue;
        }
        (*dest)->files[i] = malloc(sizeof(untrackedFile));
        copyUntrackedFile(&((*dest)->files[i]), &((*src)->files[i]));
        (*dest)->keys[i] = strdup((*src)->keys[i]);
    }
}

void rehashHashTable(hashTable* table) {
    int oldMax = (*table)->size;
    int newSize = oldMax * 2;
    hashTable oldHT = malloc(sizeof(struct hashType));
    oldHT->files = malloc(sizeof(untrackedFile) * oldMax);
    oldHT->keys = malloc(sizeof(char*) * oldMax);
    for (int i = 0; i < initialSize; i++) {
        oldHT->files[i] = NULL;
        oldHT->keys[i] = NULL;
    }
    oldHT->size = oldMax;

    copyHashTable(&oldHT, table);

    (*table)->files = malloc(sizeof(untrackedFile) * newSize);
    (*table)->keys = malloc(sizeof(char*) * newSize);
    (*table)->size = newSize;
    for (int i = 0; i < newSize; i++) {
        (*table)->files[i] = NULL;
        (*table)->keys[i] = NULL;
    }

    for (int i = 0; i < oldMax; i++) {
        if (oldHT->keys[i] != NULL) {
            addElementHT(table, oldHT->keys[i], oldHT->files[i]);
        }
    }

    free(oldHT);
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
    meow->files = malloc(sizeof(untrackedFile) * initialSize);
    meow->keys = malloc(sizeof(char*) * initialSize);
    for (int i = 0; i < initialSize; i++) {
        meow->files[i] = NULL;
        meow->keys[i] = NULL;
    }
    meow->size = initialSize;
    return (meow);
}

int asciiSum(char* string) {
    int val = 0;
    for (int i = 0; i < (int)strlen(string); i++) {
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

void addElementHT(hashTable* table, char* key, untrackedFile value) {
    int numCollisions = 0;
    bool notFull = true;
    if ((*table) == NULL) {
    }
    else {
        while ((*table)->files[hashKey(key, numCollisions, (*table)->size)] !=
               NULL) {
            if (numCollisions == (*table)->size) {
                notFull = false;
                break;
            }
            numCollisions++;
        }
        if (notFull) {
            (*table)->files[hashKey(key, numCollisions, (*table)->size)] =
                malloc(sizeof(untrackedFile));
            (*table)->files[hashKey(key, numCollisions, (*table)->size)] =
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
        freeUntrackedFile(
            &((*table)->files[hashKey(key, numCollisions, (*table)->size)]));
        (*table)->files[hashKey(key, numCollisions, (*table)->size)] =
            malloc(sizeof(untrackedFile));
        (*table)->files[hashKey(key, numCollisions, (*table)->size)] = NULL;

        (*table)->keys[hashKey(key, numCollisions, (*table)->size)] =
            malloc(sizeof(struct hashType));
        (*table)->keys[hashKey(key, numCollisions, (*table)->size)] = NULL;
    }
}

untrackedFile findValueHT(hashTable table, char* key) {
    int numCollisions = 0;
    bool found = false;
    if ((table == NULL) || !(keyExists(table, key))) {
        return (NULL);
    }
    while (table->files[hashKey(key, numCollisions, table->size)] != NULL) {
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
        return (table->files[hashKey(key, numCollisions, table->size)]);
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
                    untrackedFileToString(table->files[i]));
            }

            else if (i == (table->size - 1)) {
                printf(
                    "%s: %s }\n", table->keys[i],
                    untrackedFileToString(table->files[i]));
            }

            else {
                printf(
                    "%s: %s, ", table->keys[i],
                    untrackedFileToString(table->files[i]));
            }
        }
    }
}
