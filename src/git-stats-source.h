#pragma once
#include <obs-internal.h>
#include <obs-module.h>
#include <obs-source.h>

#include "./hashMap/include/hashMap.h"

// struct obs_source_info git_stats_source;
struct gitData {
    hashMap untracked;
    char** trackedPaths;
    int numTrackedFiles;
    long added;
    long deleted;
    int delayAmount;
};
