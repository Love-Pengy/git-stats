
#include "../include/untrackedFile.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "../../../git-diff-interface.h"
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

*/

#define MAX_LINE_LENGTH 5000

// structure to hold data for a specific untrackedFile
struct fileType {
    struct tm lastEdited;
    char* path;
    long linesAdded;
};

void copyUntrackedFile(untrackedFile* dest, untrackedFile* src) {
    if ((*src) == NULL) {
        (*dest) = NULL;
        return;
    }

    (*dest) = malloc(sizeof(struct fileType));

    //(*dest)->lastEdited = malloc(sizeof(struct tm));
    (*dest)->lastEdited.tm_sec = (*src)->lastEdited.tm_sec;
    (*dest)->lastEdited.tm_min = (*src)->lastEdited.tm_min;
    (*dest)->lastEdited.tm_hour = (*src)->lastEdited.tm_hour;
    (*dest)->lastEdited.tm_mday = (*src)->lastEdited.tm_mday;
    (*dest)->lastEdited.tm_wday = (*src)->lastEdited.tm_wday;
    (*dest)->lastEdited.tm_mon = (*src)->lastEdited.tm_mon;
    (*dest)->lastEdited.tm_year = (*src)->lastEdited.tm_year;
    (*dest)->lastEdited.tm_yday = (*src)->lastEdited.tm_yday;
    (*dest)->lastEdited.tm_isdst = (*src)->lastEdited.tm_isdst;

    (*dest)->path = malloc(sizeof(char) * (strlen((*src)->path) + 1));
    (*dest)->path = strdup((*src)->path);

    (*dest)->linesAdded = (*src)->linesAdded;
}

struct tm* getModifiedTime(char* path) {
    struct stat attr;
    stat(path, &attr);
    return (localtime(&attr.st_mtime));
}

void freeUntrackedFile(untrackedFile* file) {
    free(&((*file)->lastEdited));
    free((*file)->path);
    free(file);
    file = NULL;
}

long getLinesInFile(char* path) {
    if (path == NULL) {
        return (0);
    }
    FILE* fptr;
    errno = 0;
    char* pathCpy = malloc(sizeof(char) * 1000);
    pathCpy[0] = '\0';
    strcpy(pathCpy, path);
    expandHomeDir(&pathCpy);
    fptr = fopen(pathCpy, "r");
    if (errno) {
        printf("%s: %s\n", pathCpy, strerror(errno));
        return (0);
    }

    char buffer[MAX_LINE_LENGTH];
    long lineCount = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, fptr)) {
        lineCount += 1;
    }

    fclose(fptr);
    return (lineCount);
}

struct tm getTimeEdited(untrackedFile file) { return (file->lastEdited); }

long getLinesAdded(untrackedFile file) { return (file->linesAdded); }

untrackedFile createUntrackedFile(char* filePath) {
    untrackedFile output = malloc(sizeof(struct fileType));
    output->path = malloc(sizeof(char) * strlen(filePath));
    output->path[0] = '\0';
    snprintf(output->path, strlen(filePath) + 1, "%s", filePath);
    // output->lastEdited = malloc(sizeof(struct tm));
    output->lastEdited = *(getModifiedTime(filePath));
    output->linesAdded = getLinesInFile(filePath);
    return (output);
}

void updateUntrackedFile(untrackedFile* file) {
    if (file == NULL) {
        return;
    }
    // convert tm structs into time_t so I can use difftime
    time_t time1 = mktime(&((*file)->lastEdited));
    struct tm newTime = *(getModifiedTime((*file)->path));
    time_t time2 = mktime(&newTime);
    // if it hasnt been updates
    if (!difftime(time1, time2)) {
        return;
    }
    (*file)->lastEdited = newTime;
    (*file)->linesAdded = getLinesInFile((*file)->path);
}
/*
struct fileType {
    struct tm lastEdited;
    char* path;
    long linesAdded;
};
*/

char* untrackedFileToString(untrackedFile file) {
    if ((file == NULL) || (file->path == NULL)) {
        return ("NULL");
    }

    int stringLength = (sizeof(char) * (strlen(file->path) + 200 + 8 + 1));
    // 8 bytes for long
    // 200 bytes for tm (man page said so)
    char* buffer = malloc(stringLength);

    char timeStr[200];
    strftime(timeStr, sizeof(timeStr), "%I:%M", &(file->lastEdited));
    snprintf(
        buffer, (stringLength + 10), "{ %s: %s +%ld }", timeStr, file->path,
        file->linesAdded);

    return (buffer);
}
