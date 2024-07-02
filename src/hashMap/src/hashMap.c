#include "../include/hashMap.h"

#include <stdio.h>
#include <stdlib.h>

#include "../lib/include/hashTable.h"

struct hashMapType {
    hashTable table;
};

hashMap createHashMap(void) {
    hashMap output = malloc(sizeof(struct hashMapType));
    output->table = createHT();
    return (output);
}

long getLinesAddedHM(hashMap* map) {
    return (getLinesAddedHT(&((*map)->table)));
}

void updateValueHM(hashMap* map) { autoUpdateValuesHT(&((*map)->table)); }

void addElementHM(hashMap* map, char* key, untrackedFile value) {
    if ((*map) == NULL) {
    }
    else {
        if (!findValueHT((*map)->table, key)) {
            addElementHT(&((*map)->table), key, value);
        }
        else {
            updateValueHT(&((*map)->table), key, value);
        }
    }
}

void removeElementHM(hashMap* map, char* key) {
    if ((*map) == NULL) {
    }
    else {
        removeElementHT(&((*map)->table), key);
    }
}

untrackedFile findValueHM(hashMap map, char* key) {
    if (map == NULL) {
        return (NULL);
    }
    return (findValueHT((map)->table, key));
}

void printHM(hashMap map) {
    if (map == NULL) {
        printf("{ EMPTY }\n");
    }
    else {
        printHT(map->table);
    }
}

void freeHM(hashMap* map) {
    if (*map) {
        freeHashTable(&((*map)->table));
        free(*map);
        (*map) = NULL;
    }
}
