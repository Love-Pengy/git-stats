#include <stdlib.h>

#include "../include/boardingPassExample.h"
#include "../include/hashTable.h"

int main() {
    hashTable test = createHT();
    boardingPass test1 = createBoardingPass(rand() % 10, "Will", "1");
    boardingPass test2 = createBoardingPass(rand() % 10, "Will", "2");
    boardingPass test3 = createBoardingPass(rand() % 10, "NotWill", "3");
    addElementHT(&test, "key1", test1);
    addElementHT(&test, "laskjhf", test2);
    addElementHT(&test, "kasdlfkj", test3);
    printHT(test);
    return 0;
}
