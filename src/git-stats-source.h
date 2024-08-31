#pragma once
#include <obs-module.h>
#include <obs-source.h>
#include <time.h>

// struct obs_source_info git_stats_source;
struct gitData {
	char **trackedPaths;
	int numTrackedFiles;
  time_t* trackedRepoMTimes;
  char **untrackedFiles;
  int numUntrackedFiles;
  bool deletionEnabled;
  bool deletionSymbolEnabled;
  bool insertionEnabled;
  bool insertionSymbolEnabled;
  int delayAmount;
  long added;
  long deleted;
  long previousUntrackedAdded;
  char *overloadChar;
};
