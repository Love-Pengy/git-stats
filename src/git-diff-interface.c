#include "git-diff-interface.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <obs-module.h>
#include <obs-source.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "./support.h"

#define MAXOUTPUTSIZE 1024

bool checkInsertions(char *input)
{
	if (input == NULL) {
		return (false);
	}
	errno = 0;
	char *tmpString = bmalloc(sizeof(char) * strlen(input) + 1);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (false);
	}
	strncpy(tmpString, input, strlen(input) + 1);
	char *checker = strtok(tmpString, "insertions");
	if (checker == NULL) {
		errno = 0;
		tmpString = bmalloc(sizeof(char) * strlen(input) + 1);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
			return (false);
		}
		tmpString[0] = '\0';
		strncpy(tmpString, input, strlen(input) + 1);
		checker = strtok(tmpString, "insertion");
		if (checker == NULL) {
			bfree(tmpString);
			return (false);
		}
	}
	bfree(tmpString);
	return (true);
}

bool checkDeletions(char *input)
{
	if (input == NULL) {
		return (false);
	}
	errno = 0;
	char *tmpString = bmalloc(sizeof(char) * strlen(input) + 1);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (false);
	}
	tmpString[0] = '\0';
	strncpy(tmpString, input, strlen(input) + 1);
	char *checker = strtok(tmpString, "deletions");
	if (checker == NULL) {
		errno = 0;
		tmpString = bmalloc(sizeof(char) * strlen(input) + 1);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
			return (false);
		}
		tmpString[0] = '\0';
		strncpy(tmpString, input, strlen(input) + 1);
		checker = strtok(tmpString, "deletion");
		if (checker == NULL) {
			bfree(tmpString);
			return (false);
		}
	}
	bfree(tmpString);
	return (true);
}

void trailingNewlineDestroyer(char *victim)
{
	victim[strlen(victim) - 1] = '\0';
}

long getInsertionNumber(char *diffString)
{
	if (diffString == NULL) {
		return (0);
	}
	errno = 0;
	char *diffStringCopy = bmalloc(sizeof(char) * (strlen(diffString) + 1));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (0);
	}
	diffStringCopy[0] = '\0';
	strcpy(diffStringCopy, diffString);
	char *buff2 = NULL;
	char *startDelim = strstr(diffStringCopy, "insertions");
	if (!startDelim) {
		startDelim = strstr(diffStringCopy, "insertion");
	}
	if (!startDelim) {
		bfree(diffStringCopy);
		return (0);
	}
	char buffer[MAXOUTPUTSIZE];
	int buffSize = strlen(diffString) - strlen(startDelim);
	for (int i = 0; i < buffSize; i++) {
		buffer[i] = diffString[i];
	}
	char *amt = NULL;
	buff2 = strtok(buffer, ",");
	amt = buff2;
	while ((buff2 = strtok(NULL, ","))) {
		amt = buff2;
	}
	bfree(diffStringCopy);
	return (atol(amt));
}

long getDeletionNumber(char *diffString)
{
	if (diffString == NULL) {
		return (0);
	}
	errno = 0;
	char *diffStringCopy = bmalloc(sizeof(char) * (strlen(diffString) + 1));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (0);
	}
	diffStringCopy[0] = '\0';
	strcpy(diffStringCopy, diffString);
	char *buff2;
	char *startDelim = strstr(diffStringCopy, "deletions");
	if (!startDelim) {
		startDelim = strstr(diffStringCopy, "deletion");
	}
	if (!startDelim) {
		bfree(diffStringCopy);
		return (0);
	}
	char buffer[MAXOUTPUTSIZE];
	for (int i = 0; i < (int)(strlen(diffString) - strlen(startDelim));
	     i++) {
		buffer[i] = diffString[i];
	}
	char *amt;
	buff2 = strtok(buffer, ",");
	amt = buff2;
	while ((buff2 = strtok(NULL, ","))) {
		amt = buff2;
	}
	bfree(diffStringCopy);
	return (atol(amt));
}

char *formatEndPathChar(char *formatee)
{
	errno = 0;
	char *output = bmalloc(sizeof(char) * strlen(formatee) + 3);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (formatee);
	}
	output[0] = '\0';
	snprintf(output, (strlen(formatee) + 2), "%s%c", formatee, '/');
	return (output);
}

void expandHomeDir(char **input)
{
	char expanded[100] = "\0";
	strcpy(expanded, getHomePath());
	errno = 0;
	char *inputHold = bmalloc(sizeof(char) * (strlen((*input)) + 1));
	if (errno) {

		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return;
	}
	inputHold[0] = '\0';
	strcpy(inputHold, ((*input) + 1));
	errno = 0;
	(*input) = brealloc((*input), sizeof(char) * (strlen((*input)) +
						      strlen(expanded) + 1));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return;
	}
	(*input)[0] = '\0';
	snprintf((*input), (strlen(inputHold) + strlen(expanded) + 1), "%s%s",
		 expanded, inputHold);
	bfree(inputHold);
}

// check if path is a git dir
bool checkPath(char *path)
{
	DIR *dptr;
	char *buffer;
	char *fixedPath = NULL;

	// check if the directory exists
	errno = 0;
	buffer = bmalloc(sizeof(char) * (strlen(path) + 8));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return (false);
	}
	buffer[0] = '\0';
	if (path[strlen(path) - 1] != '/') {
		fixedPath = formatEndPathChar(path);
		strcpy(buffer, fixedPath);
	} else {
		strcpy(buffer, path);
	}
	strncat(buffer, ".git/", 6);
	if (buffer[0] == '~') {
		expandHomeDir(&buffer);
	}
	errno = 0;
	dptr = opendir(buffer);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		bfree(buffer);
		if (fixedPath) {
			bfree(fixedPath);
		}
		return (false);
	}
	closedir(dptr);
	bfree(buffer);
	if (fixedPath) {
		bfree(fixedPath);
	}
	return (true);
}
void updateTrackedFiles(struct gitData *data)
{
	if (data == NULL) {
		obs_log(LOG_INFO, "%s (%d): %s", __FILE__, __LINE__,
			"Data Struct Uninitialized");
		return;
	}
	char output[1000];

	long insertions = 0;
	long deletions = 0;
	for (int i = 0; i < data->numTrackedFiles; i++) {
		FILE *fp;
		if (!checkPath(data->trackedPaths[i])) {
			continue;
		}
		int commandLength =
			(strlen("/usr/bin/git -C ") +
			 strlen(data->trackedPaths[i]) +
			 strlen(" diff --no-pager --shortstat") + 1);

		errno = 0;
		char *command = bmalloc(sizeof(char) * commandLength);
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
			continue;
		}
		command[0] = '\0';

		snprintf(command, commandLength, "%s %s %s", "/usr/bin/git -C",
			 data->trackedPaths[i], "diff --shortstat");

		fp = popen(command, "r");
		bfree(command);

		if (fp == NULL) {
			continue;
		}
		char *fileOutput = fgets(output, sizeof(output), fp);
		pclose(fp);
		if (fileOutput) {
			bool insertionExists = checkInsertions(output);
			bool deletionExists = checkDeletions(output);
			if (insertionExists) {
				insertions += getInsertionNumber(output);
			}
			if (deletionExists) {
				deletions += getDeletionNumber(output);
			}
		}
	}

	data->added += insertions;
	data->deleted += deletions;
}

void addGitRepoDir(struct gitData *data, char *repoDirPath)
{
	char *buffer;
	DIR *dptr;
	char *fixedRepoDirPath = NULL;
	errno = 0;
	buffer = bmalloc(sizeof(char) * (strlen(repoDirPath) + 8));
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		return;
	}
	buffer[0] = '\0';

	if (repoDirPath[strlen(repoDirPath) - 1] != '/') {
		fixedRepoDirPath = formatEndPathChar(repoDirPath);
		strcpy(buffer, fixedRepoDirPath);
	} else {
		strcpy(buffer, repoDirPath);
	}
	if (buffer[0] == '~') {
		expandHomeDir(&buffer);
	}
	errno = 0;
	dptr = opendir(buffer);
	if (errno) {
		obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
			strerror(errno));
		bfree(buffer);
		return;
	}
	closedir(dptr);

	errno = 0;
	DIR *dp;
	if (fixedRepoDirPath) {
		dp = opendir(fixedRepoDirPath);
	} else {
		dp = opendir(repoDirPath);
	}
	if (errno) {
		obs_log(LOG_ERROR, "AddGitRepoDir: %s", strerror(errno));
		bfree(buffer);
		return;
	}
	errno = 0;
	struct dirent *dirStruct = readdir(dp);
	if (errno) {
		bfree(buffer);
		return;
	}

	if (strcmp(dirStruct->d_name, ".") && strcmp(dirStruct->d_name, "..") &&
	    (dirStruct != NULL)) {
		errno = 0;
		char *tmpFilePath = NULL;
		if (!fixedRepoDirPath) {
			tmpFilePath = bmalloc(sizeof(char) *
					      (strlen(dirStruct->d_name) +
					       strlen(repoDirPath) + 2));
		} else {
			tmpFilePath = bmalloc(sizeof(char) *
					      (strlen(dirStruct->d_name) +
					       strlen(fixedRepoDirPath) + 2));
		}
		if (errno) {
			obs_log(LOG_ERROR, "%s (%d): %s", __FILE__, __LINE__,
				strerror(errno));
			bfree(buffer);
			return;
		}
		tmpFilePath[0] = '\0';
		if (!fixedRepoDirPath) {
			snprintf(tmpFilePath,
				 (strlen(repoDirPath) +
				  strlen(dirStruct->d_name) + 1),
				 "%s%s", repoDirPath, dirStruct->d_name);
		} else {
			snprintf(tmpFilePath,
				 (strlen(fixedRepoDirPath) +
				  strlen(dirStruct->d_name) + 1),
				 "%s%s", fixedRepoDirPath, dirStruct->d_name);
		}
		if (checkPath(tmpFilePath)) {
			if (!checkRepoExists(data->trackedPaths,
					     data->numTrackedFiles,
					     tmpFilePath)) {
				errno = 0;
				data->trackedPaths[data->numTrackedFiles] =
					bmalloc(sizeof(char) *
						(strlen(tmpFilePath) + 1));
				if (errno) {
					obs_log(LOG_ERROR, "%s (%d): %s",
						__FILE__, __LINE__,
						strerror(errno));
					bfree(tmpFilePath);
					bfree(buffer);
					return;
				}
				strncpy(data->trackedPaths[data->numTrackedFiles],
					tmpFilePath, (strlen(tmpFilePath) + 1));
				data->numTrackedFiles++;
			}
		}
		if (tmpFilePath) {
			bfree(tmpFilePath);
		}
	}
	while ((dirStruct = readdir(dp))) {
		if (strcmp(dirStruct->d_name, ".") &&
		    strcmp(dirStruct->d_name, "..")) {
			errno = 0;
			char *tmpFilePath = NULL;
			if (!fixedRepoDirPath) {
				tmpFilePath =
					bmalloc(sizeof(char) *
						(strlen(dirStruct->d_name) +
						 strlen(repoDirPath) + 2));
			} else {
				tmpFilePath =
					bmalloc(sizeof(char) *
						(strlen(dirStruct->d_name) +
						 strlen(fixedRepoDirPath) + 2));
			}
			if (errno) {
				obs_log(LOG_ERROR, "%s (%d): %s", __FILE__,
					__LINE__, strerror(errno));
				continue;
			}
			tmpFilePath[0] = '\0';
			if (!fixedRepoDirPath) {
				snprintf(tmpFilePath,
					 (strlen(repoDirPath) +
					  strlen(dirStruct->d_name) + 1),
					 "%s%s", repoDirPath,
					 dirStruct->d_name);
			} else {
				snprintf(tmpFilePath,
					 (strlen(fixedRepoDirPath) +
					  strlen(dirStruct->d_name) + 1),
					 "%s%s", fixedRepoDirPath,
					 dirStruct->d_name);
			}
			if (checkPath(tmpFilePath)) {
				if (!checkRepoExists(data->trackedPaths,
						     data->numTrackedFiles,
						     tmpFilePath)) {
					errno = 0;
					data->trackedPaths[data->numTrackedFiles] =
						bmalloc(sizeof(char) *
							(strlen(tmpFilePath) +
							 1));
					if (errno) {
						obs_log(LOG_ERROR,
							"%s (%d): %s", __FILE__,
							__LINE__,
							strerror(errno));
						bfree(tmpFilePath);
						continue;
					}
					strncpy(data->trackedPaths
							[data->numTrackedFiles],
						tmpFilePath,
						(strlen(tmpFilePath) + 1));
					data->numTrackedFiles++;
				}
			}
			if (tmpFilePath) {
				bfree(tmpFilePath);
			}
		}
	}
	if (dp) {
		closedir(dp);
	}
	if (fixedRepoDirPath) {
		bfree(fixedRepoDirPath);
	}
	bfree(buffer);
}

char *checkInvalidRepos(char **paths, int numPaths)
{
	for (int i = 0; i < numPaths; i++) {
		if (!checkPath(paths[i])) {
			return (paths[i]);
		}
	}
	return (NULL);
}
