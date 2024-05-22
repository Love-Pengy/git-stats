#include "../include/hashTable.h"

int main() {
    hashTable test = createHT();
    addElementHT(&test, "key1", 1982);
    addElementHT(&test, "laskjhf", 2001);
    addElementHT(&test, "kasdlfkj", 2002);
    addElementHT(&test, "keyalskdj2ck", 2003);
    addElementHT(&test, "key2", 2091);
    removeElementHT(&test, "key2");
    removeElementHT(&test, "keyalskdj2ck");
    removeElementHT(&test, "key1");
    removeElementHT(&test, "laskjhf");
    removeElementHT(&test, "kasdlfkj");
    printHT(test);
    return 0;
}
