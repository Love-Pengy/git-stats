#include "../include/untrackedFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "untrackedFile.h"

/*
tm struct{
       int8_t tm_sec
       int8_t tm_min
       int8_t tm_hour
       int8_t tm_mday
       int8_t tm_wday
       int8_t tm_mon
       int16_t tm_year
       int16_t tm_yday
       int16_t tm_isdst

 }

void getFileCreationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
    printf("Last modified time: %s", ctime(&attr.st_mtime));
}
*/

#define MAX_LINE_LENGTH 5000
// structure to hold data for a specific untrackedFile

struct fileType {
    struct tm lastEdited;
    char* path;
    int linesAdded;
};

struct tm* getModifiedTime(char* path) {
    struct stat attr;
    stat(path, &attr);
    return (localtime(&attr.st_mtime));
}

int getLinesInFile(char* path) {
    FILE* fptr;
    fptr = fopen(path, "r");
    char buffer[5001];
    int lineCount = 0;
    while (fgets(buffer, 5001, fptr)) {
        lineCount += 1;
    }
    fclose(fptr);
    return (lineCount);
}

struct tm getTimeEdited(untrackedFile file) { return (file->lastEdited); }

int getLinesAdded(untrackedFile file) { return (file->linesAdded); }

untrackedFile createUntrackedFile(char* filePath) {
    untrackedFile output = malloc(sizeof(struct fileType));
    output->path = malloc(sizeof(char) * strlen(filePath));
    output->path[0] = '\0';
    snprintf(output->path, strlen(filePath) + 1, "%s", filePath);
    output->lastEdited = malloc(sizeof(struct tm));
    output->lastEdited = *(getModifiedTime(filePath));

    return (output);
}

char* boardingPassToString(boardingPass pass) {
    if ((pass == NULL) || (pass->idNum == -1)) {
        return ("NULL");
    }
    char* buffer = malloc(
        sizeof(char) * (strlen(pass->firstName) + strlen(pass->lastName) + 7));

    snprintf(
        buffer, (strlen(pass->firstName) + strlen(pass->lastName) + 7 + 5),
        "{ %d, %s %s }", pass->idNum, pass->firstName, pass->lastName);
    return (buffer);
}
