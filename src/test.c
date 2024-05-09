#include <stdio.h>
#include <stdlib.h>

char* test(void) {
    char* test;
    test = "asldkfj";
}

int main() {
    char* something = test();
    printf("%s\n", something);
    return 0;
}
