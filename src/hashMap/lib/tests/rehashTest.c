#include <stdio.h>
#include <stdlib.h>

#include "../include/hashTable.h"

int main() {
    hashTable test = createHT();
    boardingPass b1 = createBoardingPass(rand() % 1000, "b1First", "b1Last");
    boardingPass b2 = createBoardingPass(rand() % 1000, "b2First", "b2Last");
    boardingPass b3 = createBoardingPass(rand() % 1000, "b3First", "b3Last");
    boardingPass b4 = createBoardingPass(rand() % 1000, "b4First", "b4Last");
    boardingPass b5 = createBoardingPass(rand() % 1000, "b5First", "b5Last");
    addElementHT(&test, "key1", b1);
    addElementHT(&test, "key2", b2);
    addElementHT(&test, "key3", b3);
    printHT(test);
    rehashHashTable(&test);
    addElementHT(&test, "key4", b4);
    addElementHT(&test, "key5", b5);
    printHT(test);
    printf("%s\n", boardingPassToString(findValueHT(test, "key1")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key2")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key3")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key4")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key5")));
    printf(
        "%s\n",
        boardingPassToString(findValueHT(test, "somethingthatisn'tinit:3")));
    removeElementHT(&test, "key1");
    removeElementHT(&test, "key2");
    removeElementHT(&test, "key3");
    removeElementHT(&test, "key4");
    removeElementHT(&test, "key5");
    removeElementHT(&test, "somethingthatdoesn'texist");
    printHT(test);
    printf("\n\n");
    printf("%s\n", boardingPassToString(findValueHT(test, "key1")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key2")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key3")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key4")));
    printf("%s\n", boardingPassToString(findValueHT(test, "key5")));
    printf(
        "%s\n",
        boardingPassToString(findValueHT(test, "somethingthatisn'tinit:3")));
    freeHashTable(&test);
    printHT(test);
    return 0;
}
