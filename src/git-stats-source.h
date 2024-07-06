#pragma once
#include <obs-module.h>
#include <obs-source.h>

#include "./hashMap/include/hashMap.h"

// struct obs_source_info git_stats_source;
struct gitData {
    hashMap untracked;
    char** trackedPaths;
    int numTrackedFiles;
    bool insertionEnabled;
    long added;
    bool deletionEnabled;
    long deleted;
    int delayAmount;
    char* overloadChar;
};
