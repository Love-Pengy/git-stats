#include <stdio.h>
#include <stdlib.h>

int main() {
    int* arr = malloc(sizeof(int) * 10);
    arr[0] = NULL;
    printf("%d\n", arr[0]);
    return 0;
}
