#include <stdio.h>
#include <stdlib.h>

#include "../include/hashMap.h"
int main() {
    hashMap hm = createHashMap();
    removeElementHM(&hm, "meow");
    boardingPass b1 = createBoardingPass(rand() % 1000, "b1First", "b1Last");
    boardingPass b2 = createBoardingPass(rand() % 1000, "b2First", "b2Last");
    boardingPass b3 = createBoardingPass(rand() % 1000, "b3First", "b3Last");
    boardingPass b4 = createBoardingPass(rand() % 1000, "b4First", "b4Last");
    addElementHM(&hm, "b1", b1);
    addElementHM(&hm, "b2", b2);
    addElementHM(&hm, "b3", b3);
    printf("B1-B3: ");
    printHM(hm);
    removeElementHM(&hm, "b1");
    printf("B2-B3: ");
    printHM(hm);
    addElementHM(&hm, "b2", b4);
    printf("B2 updated: ");
    printHM(hm);
}
