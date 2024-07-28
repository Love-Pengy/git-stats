#include <stdio.h>

#include "../include/hashTable.h"

int main()
{
	printf("Hash 1 => %d\n", hashKey("test1"));
	printf("Hash 2 => %d\n", hashKey("test1"));
	printf("Hash 3 => %d\n", hashKey(" "));
	printf("Hash 4 => %d\n", hashKey("\n"));
	printf("Hash 5 => %d\n", hashKey("as;lkdhf"));
	return 0;
}
