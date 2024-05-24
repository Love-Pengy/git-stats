#pragma once
#include "./git-stats-source.h"
#include "hashMap/lib/include/untrackedFile.h"
void updateTrackedFiles(struct gitData*);
void createUntrackedFilesHM(struct gitData*);
