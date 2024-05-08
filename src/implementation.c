#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

void checkAllPaths(int numPaths, char** paths) {
    DIR* dptr;
    char* buffer;
    for (int i = 1; i < numPaths; i++) {
        // check if the directory exists
        errno = 0;
        buffer = malloc(sizeof(char) * (strlen(paths[i]) + 5));
        buffer[0] = '\0';
        strcpy(buffer, paths[i]);
        snprintf(buffer, (strlen(paths[i]) + 7), "%s%s", paths[i], ".git/");
        dptr = opendir(buffer);

        if (errno) {
            perror("File is not a git repository");
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

    // do this for all items within the structure
    fp = popen(
        "/usr/bin/git -C /home/Bee/Projects/git-stats/ diff --shortstat", "r");

    assert(fp != NULL);

    bool validOutput;
    while (fgets(output, sizeof(output), fp)) {
        validOutput = checkInsertions(output);
        if (validOutput) {
            printf("%s\n", output);
        }
        else {
            assert(validOutput != true);
            exit(EXIT_FAILURE);
        }
    }
    pclose(fp);
}
