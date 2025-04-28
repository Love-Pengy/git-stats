#include "support.h"

#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <obs-module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <errno.h>

// extracts first unicode character from a given string
char *extract_unicode(const char *input)
{
  if(!input){
    return(NULL);
  }
	errno = 0;
	char *buff = bmalloc(sizeof(char) * MB_CUR_MAX);
	buff[0] = '\0';
	errno = 0;
	char *inputCpy = bmalloc(sizeof(char) * (strlen(input) + 2));
	inputCpy[0] = '\0';
	strncpy(inputCpy, input, strlen(input) + 1);
	char32_t specChar;
	mbstate_t mbs;
	char *locale = setlocale(LC_ALL, "");
	if (!locale) {
		warning("%s (%d): %s", __FILE__, __LINE__, strerror(errno));
		bfree(buff);
		return (NULL);
	}
	memset(&mbs, 0, sizeof(mbs));

	size_t status = mbrtoc32(&specChar, inputCpy, 16, &mbs);
	if (status == (size_t)-1 || status == (size_t)-2) {
		warning("%s (%d): %s", __FILE__, __LINE__, strerror(errno));
		return (NULL);
	}
	int size = c32rtomb(buff, specChar, &mbs);
	if (size < 0) {
		warning("%s (%d): %s", __FILE__, __LINE__, strerror(errno));
		return (NULL);
	}
	// ASSUMED: if size = 1 then its a normal character not a nerdfont character
	else if (size == 1) {
		bfree(buff);
		inputCpy[1] = '\0';
		return (inputCpy);
	}
	// ensure that string gets terminated in the correct spot
	buff[size] = '\0';
	bfree(inputCpy);
	return (buff);
}

// pad string with spaces on left to fill size space
void pad_string_left(char* input, int size){
  if(!input){
    return;
  }
  char* buff = bmalloc(sizeof(char) * size);
  buff[size] = '\0';
  (void)memset(buff, ' ', size); 
  (void)memcpy(buff+(size-strlen(input)), input, strlen(input));  
  (void)memcpy(input, buff, size);
  bfree(buff);
}

// pad string with spaces on right to fill size space
void pad_string_right(char* input, int size){
  memset(input+strlen(input), ' ', size-strlen(input)); 
};

