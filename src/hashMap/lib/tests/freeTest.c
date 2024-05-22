#include <stdio.h>
#include <stdlib.h>

// Testing to see if I can get integers from stoofs
int main() {
    int test = 1100;
    char* buffer = malloc(sizeof(char) * 100);
    snprintf(buffer, 100, "%d", test);
    free(buffer);
    buffer = malloc(sizeof(char) * 100);
    snprintf(buffer, 100, "%d", test);
    printf("%s\n", buffer);
    return 0;
}
