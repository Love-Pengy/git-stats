// #include "../hashTable.h"
#include <stdio.h>
#include <time.h>

#include "../untrackedFile.h"

int main() {
    untrackedFile b1 =
        createUntrackedFile("/home/Bee/Projects/BeeLib/hashTable/src/test.c");
    untrackedFile b2 = createUntrackedFile(
        "/home/Bee/Projects/git-stats/src/hashTable/tests/"
        "untrackedFilesTest.c");
    updateUntrackedFile(&b1);
    updateUntrackedFile(&b2);
    printf("LINES ADDED: %ld %ld\n", getLinesAdded(b1), getLinesAdded(b2));
    char timeStr[200];
    strftime(
        timeStr, sizeof(timeStr), "%c",
        (getModifiedTime(getUntrackedFilePath(b1))));
    printf("TIME EDITED 1: %s\n", timeStr);
    strftime(
        timeStr, sizeof(timeStr), "%c",
        (getModifiedTime(getUntrackedFilePath(b2))));
    printf("TIME EDITED 2: %s\n", timeStr);
    printf("B1: %s\n", untrackedFileToString(b1));
    printf("B2: %s\n", untrackedFileToString(b2));
}
