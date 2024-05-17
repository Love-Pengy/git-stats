#include <stdio.h>
#include <stdlib.h>

#include "../hashTable.h"

int main() {
    hashTable test = createHT();
    untrackedFile b1 = createUntrackedFile("/home/Bee/Projects/BeeLib/");
    untrackedFile b2 = createUntrackedFile("/home/Bee/Projects/CatBot/");
    untrackedFile b3 = createUntrackedFile("/home/Bee/Projects/CloudToLocal/");
    untrackedFile b4 = createUntrackedFile("/home/Bee/Projects/LilLilacLush/");
    untrackedFile b5 = createUntrackedFile("/home/Bee/Projects/git-stats/");
    addElementHT(&test, "key1", b1);
    addElementHT(&test, "key2", b2);
    addElementHT(&test, "key3", b3);
    printHT(test);
    rehashHashTable(&test);
    addElementHT(&test, "key4", b4);
    addElementHT(&test, "key5", b5);
    printHT(test);
    printf("%s\n", untrackedFileToString(findValueHT(test, "key1")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key2")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key3")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key4")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key5")));
    printf(
        "%s\n",
        untrackedFileToString(findValueHT(test, "somethingthatisn'tinit:3")));
    removeElementHT(&test, "key1");
    removeElementHT(&test, "key2");
    removeElementHT(&test, "key3");
    removeElementHT(&test, "key4");
    removeElementHT(&test, "key5");
    removeElementHT(&test, "somethingthatdoesn'texist");
    printHT(test);
    printf("\n\n");
    printf("%s\n", untrackedFileToString(findValueHT(test, "key1")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key2")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key3")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key4")));
    printf("%s\n", untrackedFileToString(findValueHT(test, "key5")));
    printf(
        "%s\n",
        untrackedFileToString(findValueHT(test, "somethingthatisn'tinit:3")));
    freeHashTable(&test);
    printHT(test);
    return 0;
}
