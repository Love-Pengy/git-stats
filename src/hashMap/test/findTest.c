#include <stdio.h>
#include <stdlib.h>

#include "../include/hashMap.h"
// #include "../lib/src/boardingPassExample.c"

int main() {
    hashMap hm = createHashMap();
    removeElementHM(&hm, "meow");
    boardingPass b1 = createBoardingPass(rand() % 1000, "b1First", "b1Last");
    boardingPass b2 = createBoardingPass(rand() % 1000, "b2First", "b2Last");
    boardingPass b3 = createBoardingPass(rand() % 1000, "b3First", "b3Last");
    addElementHM(&hm, "b1", b1);
    addElementHM(&hm, "b2", b2);
    addElementHM(&hm, "b3", b3);
    printf("%s\n", boardingPassToString(findValueHM(hm, "b3")));
}
