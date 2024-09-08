#pragma once
#include <obs-module.h>
#include <obs-source.h>
#include <time.h>

#define MAXNUMPATHS 100
//int MAXNUMPATHS = MNUMPATHS;

struct gitData {
	char **trackedPaths;
	int numTrackedFiles;
  time_t trackedRepoMTimes[MAXNUMPATHS];
  int prevAddedValues_Tracked[MAXNUMPATHS];
  int prevDeletedValues_Tracked[MAXNUMPATHS];
  char **untrackedFiles;
  int numUntrackedFiles;
  time_t untrackedRepoMTimes[MAXNUMPATHS];
  int prevAddedValues_Untracked[MAXNUMPATHS];
  long previousUntrackedAdded;
  bool deletionEnabled;
  bool deletionSymbolEnabled;
  bool insertionEnabled;
  bool insertionSymbolEnabled;
  int delayAmount;
  long added;
  long deleted;
  char *overloadChar;
};
