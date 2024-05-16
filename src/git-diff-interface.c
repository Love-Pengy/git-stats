#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

bool UNTRACKED_FILES = true;

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
    char* diffStringCopy = malloc(sizeof(char) * strlen(diffString));
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
    char* diffStringCopy = malloc(sizeof(char) * strlen(diffString));
    diffStringCopy[0] = '\0';
    char* buffer;
    strcpy(diffStringCopy, diffString);
    buffer = strtok(diffStringCopy, ",");
    buffer = strtok(NULL, ",");
    buffer = strtok(NULL, ",");

    output = atol(buffer);

    return (output);
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
    for (int i = 1; i < numPaths; i++) {
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
            exit(EXIT_FAILURE);
        }

        closedir(dptr);
    }
}

int main(int argc, char* argv[]) {
    // in the case that we just called the executable
    assert(argc != 1);

    FILE* fp;
    char output[1000];

    fflush(stdout);

    checkAllPaths(argc, argv);

    for (int i = 1; i < argc; i++) {
        // do this for all items within the structure::
        int commandLength =
            (strlen("/usr/bin/git -C ") + strlen(argv[i]) +
             strlen(" diff --no-pager --shortstat") + 1);

        char* command = malloc(sizeof(char) * commandLength);
        command[0] = '\0';

        snprintf(
            command, commandLength, "%s %s %s", "/usr/bin/git -C", argv[i],
            "diff --shortstat");

        fp = popen(command, "r");

        assert(fp != NULL);

        bool validOutput;
        long insertions = 0;
        long deletions = 0;
        while (fgets(output, sizeof(output), fp)) {
            validOutput = checkInsertions(output);
            if (validOutput) {
                insertions += getInsertionNumber(output);
                deletions += getDeletionNumber(output);
                printf("%ld, %ld\n", insertions, deletions);
            }
            else {
                assert(validOutput != true);
                exit(EXIT_FAILURE);
            }
        }
        pclose(fp);
    }
}
