#include "support.h"
#include <fcntl.h>

#include <locale.h>
#include <obs-module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <errno.h>

#include "git-diff-interface.h"
#include "git-stats-source.h"
const char *PLUGIN_NAME = "git-stats";
const char *PLUGIN_VERSION = "0.0.2";
int MAXNUMPATHS = 100;

void obs_log(int log_level, const char *format, ...)
{
	size_t length = 4 + strlen(PLUGIN_NAME) + strlen(format);

	char *template = bmalloc(length + 1);

	snprintf(template, length, "[%s] %s", PLUGIN_NAME, format);

	va_list(args);

	va_start(args, format);
	blogva(log_level, template, args);
	va_end(args);

	bfree(template);
}

char *ltoa(long input)
{
	// get a given value by dividing by a multiple of 10
	//  check if it's the end by first getting the lenght by seeing if it
	//  divides by a multiple without being a fraction
	long currProduct = 1;
	int size = 0;
	errno = 0;
	char *output = bmalloc(sizeof(char) * 22);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (NULL);
	}
	bool entered = false;
	output[0] = '\0';
	if (input == 0) {
		output[0] = '0';
		output[1] = '\0';
	}
	while (!((input / currProduct) < 1)) {
		if (currProduct != 1) {
			entered = true;
		}
		size++;
		currProduct *= 10;
	}
	currProduct = 1;
	for (int i = 0; i < size; i++) {
		// 0th index is highest one
		// add 48 because of ascii table
		output[i] = (char)((
			((int)(input / powl(10, ((size - 1) - i))) % 10) + 48));
		currProduct *= 10;
	}
	output[size] = '\0';
	if (!entered) {
		output[0] = (char)(input + 48);
		output[1] = '\0';
	}
	return (output);
}

char *getHomePath(void)
{
	if (getenv("HOME") == NULL) {
		return (NULL);
	}
	return (getenv("HOME"));
}

bool checkRepoExists(char **repos, int amtRepos, char *checkPath)
{
	if (repos == NULL || amtRepos == 0) {
		return (false);
	}
	for (int i = 0; i < amtRepos; i++) {
		// resolve to absolute path so comparison is easy :3
		if (repos[i][0] == '~') {
			expandHomeDir(&repos[i]);
		}
		if (checkPath[0] == '~') {
			expandHomeDir(&checkPath);
		}
		if (!strcmp(repos[i], checkPath)) {
			return (true);
		}
	}
	return (false);
}

// extracts first unicode character from a given string
char *extractUnicode(const char *input)
{
	errno = 0;
	char *buff = bmalloc(sizeof(char) * MB_CUR_MAX);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (NULL);
	}
	buff[0] = '\0';
	errno = 0;
	char *inputCpy = bmalloc(sizeof(char) * (strlen(input) + 2));
	if (errno) {

		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (NULL);
	}
	inputCpy[0] = '\0';
	strncpy(inputCpy, input, strlen(input) + 1);
	char32_t specChar;
	mbstate_t mbs;
	char *locale = setlocale(LC_ALL, "");
	if (!locale) {
		obs_log(LOG_WARNING, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		bfree(buff);
		return (NULL);
	}
	memset(&mbs, 0, sizeof(mbs));

	size_t status = mbrtoc32(&specChar, inputCpy, 16, &mbs);
	if (status == (size_t)-1 || status == (size_t)-2) {
		obs_log(LOG_WARNING, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (NULL);
	}
	int size = c32rtomb(buff, specChar, &mbs);
	if (size < 0) {
		obs_log(LOG_WARNING, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
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

//check lock status of a file true = locked, false = unlocked
bool checkLockStatus(char *path)
{
	char *pathCpy = bmalloc(sizeof(char)  * strlen(path) + 1);
	pathCpy[0] = '\0';
	strncpy(pathCpy, path, strlen(path) + 1);
	if (pathCpy[0] == '~') {
		expandHomeDir(&pathCpy);
	}
	struct flock lock;
	memset(&lock, 0, sizeof(struct flock));
	int fd = open(pathCpy, O_RDWR);
	fcntl(fd, F_GETLK, &lock);
	if (lock.l_type != F_UNLCK) {
		bfree(pathCpy);
		return (true);
	}
    bfree(pathCpy);
	return (false);
}

bool checkUntrackedFileLock(struct gitData *info)
{
	for (int i = 0; i < info->numUntrackedFiles; i++) {
		if (checkLockStatus(info->untrackedFiles[i])) {
			return (true);
		}
	}
	return (false);
}
