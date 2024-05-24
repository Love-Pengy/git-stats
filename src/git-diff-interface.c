#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// #include "./git-stats-source.h"
#include "./hashMap/include/hashMap.h"
#include "./hashMap/lib/include/untrackedFile.h"
#include "git-diff-interface.h"

bool checkInsertions(char* input) {
    char* tmpString = malloc(sizeof(char) * strlen(input) + 1);
    tmpString[0] = '\0';
    strncpy(tmpString, input, strlen(input) + 1);
    char* checker = strtok(tmpString, "insertions");
    if (checker == NULL) {
        free(tmpString);
        return (false);
    }
    free(tmpString);
    return (true);
}

void trailingSpaceDestroyer(char* victim) { victim[strlen(victim) - 1] = '\0'; }

void trailingNewlineDestroyer(char* victim) {
    victim[strlen(victim) - 1] = '\0';
}

long getInsertionNumber(char* diffString) {
    long output = 0;
    char* diffStringCopy = malloc(sizeof(char) * (strlen(diffString) + 1));
    diffStringCopy[0] = '\0';
    char* buffer;
    strcpy(diffStringCopy, diffString);
    buffer = strtok(diffStringCopy, ",");
    buffer = strtok(NULL, ",");

    output = atol(buffer);

    return (output);
}

long getDeletionNumber(char* diffString) {
    long output = 0;
    char* diffStringCopy = malloc(sizeof(char) * (strlen(diffString) + 1));
    diffStringCopy[0] = '\0';
    char* buffer;
    strcpy(diffStringCopy, diffString);
    buffer = strtok(diffStringCopy, ",");
    buffer = strtok(NULL, ",");
    buffer = strtok(NULL, ",");

    output = atol(buffer);

    return (output);
}

void createUntrackedFilesHM(struct gitData* data) {
    FILE* fp;
    char filename[1000];
    char entirePath[1000];

    for (int i = 0; i < data->numTrackedFiles; i++) {
        int commandLength =
            (strlen("/usr/bin/git -C ") + strlen(data->trackedPaths[i]) +
             strlen(" ls-files --others --exclude-standard") + 1);
        char* command = malloc(sizeof(char) * commandLength);
        command[0] = '\0';

        snprintf(
            command, commandLength + 1, "%s %s %s", "/usr/bin/git -C",
            data->trackedPaths[i], "ls-files --others --exclude-standard");

        fp = popen(command, "r");
        if (fp == NULL) {
            continue;
        }

        while (fgets(filename, sizeof(filename), fp)) {
            snprintf(
                entirePath,
                (strlen(filename) + strlen(data->trackedPaths[i]) + 1), "%s%s",
                data->trackedPaths[i], filename);
            addElementHM(
                &(data->untracked), entirePath, createUntrackedFile(filename));
        }
        pclose(fp);
    }

    fflush(stdout);
}

char* formatEndPathChar(char* formatee) {
    char* output = malloc(sizeof(char) * strlen(formatee) + 1);
    output[0] = '\0';
    snprintf(output, (strlen(formatee) + 2), "%s%c", formatee, '/');
    return (output);
}

void checkAllPaths(int numPaths, char** paths) {
    DIR* dptr;
    char* buffer;
    for (int i = 0; i < numPaths; i++) {
        // check if the directory exists
        errno = 0;
        buffer = malloc(sizeof(char) * (strlen(paths[i]) + 5));
        buffer[0] = '\0';

        if (paths[i][strlen(paths[i]) - 1] != '/') {
            paths[i] = formatEndPathChar(paths[i]);
        }
        strcpy(buffer, paths[i]);
        snprintf(buffer, (strlen(paths[i]) + 7), "%s%s", paths[i], ".git/");
        dptr = opendir(buffer);
        if (errno) {
            printf("%s: Is Not A Git Repository\n", buffer);
            // exit(EXIT_FAILURE);
        }
        closedir(dptr);
    }
}

void updateTrackedFiles(struct gitData* data) {
    FILE* fp;
    char output[1000];

    fflush(stdout);

    checkAllPaths(data->numTrackedFiles, data->trackedPaths);

    long insertions = 0;
    long deletions = 0;
    for (int i = 1; i < data->numTrackedFiles; i++) {
        // do this for all items within the structure::
        int commandLength =
            (strlen("/usr/bin/git -C ") + strlen(data->trackedPaths[i]) +
             strlen(" diff --no-pager --shortstat") + 1);

        char* command = malloc(sizeof(char) * commandLength);
        command[0] = '\0';

        snprintf(
            command, commandLength, "%s %s %s", "/usr/bin/git -C",
            data->trackedPaths[i], "diff --shortstat");

        fp = popen(command, "r");

        if (fp == NULL) {
            continue;
        }
        // assert(fp != NULL);

        bool validOutput;
        while (fgets(output, sizeof(output), fp)) {
            validOutput = checkInsertions(output);
            if (validOutput) {
                insertions += getInsertionNumber(output);
                deletions += getDeletionNumber(output);
                printf("%ld, %ld\n", insertions, deletions);
            }
            else {
                // assert(validOutput != true);
                pclose(fp);
                continue;
                // exit(EXIT_FAILURE);
            }
        }
        pclose(fp);
    }
    if (data->untracked != NULL) {
        updateValueHM(&(data->untracked));
        data->added += getLinesAddedHM(&(data->untracked));
        printf("DO WE GET DOWN HERE?\n");
    }
    data->added += insertions;
    data->deleted += deletions;
}
