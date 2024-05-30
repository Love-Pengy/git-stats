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
#include "./support.h"
#include "git-diff-interface.h"

bool checkInsertions(char* input) {
    if (input == NULL) {
        return (false);
    }
    char* tmpString = malloc(sizeof(char) * strlen(input) + 1);
    strncpy(tmpString, input, strlen(input) + 1);
    char* checker = strtok(tmpString, "insertions");
    if (checker == NULL) {
        tmpString = malloc(sizeof(char) * strlen(input) + 1);
        tmpString[0] = '\0';
        strncpy(tmpString, input, strlen(input) + 1);
        checker = strtok(tmpString, "insertion");
        if (checker == NULL) {
            free(tmpString);
            return (false);
        }
    }
    free(tmpString);
    return (true);
}
bool checkDeletions(char* input) {
    if (input == NULL) {
        return (false);
    }
    char* tmpString = malloc(sizeof(char) * strlen(input) + 1);
    tmpString[0] = '\0';
    strncpy(tmpString, input, strlen(input) + 1);
    char* checker = strtok(tmpString, "deletions");
    if (checker == NULL) {
        tmpString = malloc(sizeof(char) * strlen(input) + 1);
        tmpString[0] = '\0';
        strncpy(tmpString, input, strlen(input) + 1);
        checker = strtok(tmpString, "deletion");
        if (checker == NULL) {
            free(tmpString);
            return (false);
        }
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
    printf("got here\n");
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
            trailingNewlineDestroyer(filename);
            snprintf(
                entirePath,
                (strlen(filename) + strlen(data->trackedPaths[i]) + 1), "%s%s",
                data->trackedPaths[i], filename);
            addElementHM(
                &(data->untracked), entirePath,
                createUntrackedFile(entirePath));
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

void expandHomeDir(char** input) {
    char expanded[100] = "\0";
    strcpy(expanded, getHomePath());
    char* inputHold = malloc(sizeof(char) * (strlen((*input)) + 1));
    inputHold[0] = '\0';
    strcpy(inputHold, ((*input) + 1));
    (*input) = malloc(sizeof(char) * (strlen((*input)) + strlen(expanded) + 1));
    (*input)[0] = '\0';
    snprintf(
        (*input), (strlen(inputHold) + strlen(expanded) + 1), "%s%s", expanded,
        inputHold);
}

// check if path is a git dir
bool checkPath(char* path) {
    DIR* dptr;
    char* buffer;

    // check if the directory exists
    errno = 0;
    buffer = malloc(sizeof(char) * (strlen(path) + 5));
    buffer[0] = '\0';

    if (path[strlen(path) - 1] != '/') {
        path = formatEndPathChar(path);
    }
    strcpy(buffer, path);
    snprintf(buffer, (strlen(path) + 7), "%s%s", path, ".git/");
    if (buffer[0] == '~') {
        expandHomeDir(&buffer);
    }
    dptr = opendir(buffer);
    if (errno) {
        closedir(dptr);
        return (false);
    }
    closedir(dptr);
    return (true);
}

void updateTrackedFiles(struct gitData* data) {
    FILE* fp;
    char output[1000];

    fflush(stdout);

    long insertions = 0;
    long deletions = 0;
    for (int i = 0; i < data->numTrackedFiles; i++) {
        if (!checkPath(data->trackedPaths[i])) {
            continue;
        }
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

        if (fgets(output, sizeof(output), fp)) {
            bool insertionExists = checkInsertions(output);
            bool deletionExists = checkDeletions(output);
            if (insertionExists) {
                insertions += getInsertionNumber(output);
            }
            if (deletionExists) {
                deletions += getDeletionNumber(output);
            }
        }
        pclose(fp);
    }
    if (data->untracked != NULL) {
        updateValueHM(&(data->untracked));
        data->added += getLinesAddedHM(&(data->untracked));
    }
    data->added += insertions;
    data->deleted += deletions;
}
