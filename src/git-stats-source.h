#pragma once
#include <obs-module.h>
#include <obs-source.h>

// TODO: Change out for actual data source. sem, mutex, thread,
// and an atomic instant refresh bool 
struct git_stats_data {
	char **trackedPaths;
	int numTrackedFiles;
	bool insertionEnabled;
	bool insertionSymbolEnabled;
	long added;
	bool deletionEnabled;
	bool deletionSymbolEnabled;
	long deleted;
	int delayAmount;
	char *overloadChar;
	int numUntrackedFiles;
	char **untrackedFiles;
	long previousUntrackedAdded;
};
