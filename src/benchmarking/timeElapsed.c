#include "timeElapsed.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <obs-module.h>
bench *startTimer(void)
{
	errno = 0;
	bench *benchmarker = bmalloc(sizeof(struct timeObj));
	if (errno) {
		printf("%s (%d): %s\n", __FILE__, __LINE__, strerror(errno));
		return (NULL);
	}
	benchmarker->endTime.tv_usec = 0;
	benchmarker->endTime.tv_sec = 0;
	benchmarker->startTime.tv_usec = 0;
	benchmarker->startTime.tv_sec = 0;
	errno = 0;
	gettimeofday(&(benchmarker->startTime), NULL);
	if (errno) {
		printf("%s (%d): %s\n", __FILE__, __LINE__, strerror(errno));
		return (NULL);
	}
	return (benchmarker);
}

void endTimer(bench *benchmarker)
{
	if (!benchmarker) {
		return;
	}
	gettimeofday(&(benchmarker->endTime), NULL);
}

uint64_t getElapsedTimeUs(bench *benchParam)
{
	if (!benchParam) {
		return (0);
	}
	bench benchmarker = *benchParam;
	return ((benchmarker.endTime.tv_sec * (uint64_t)1000000 +
		 benchmarker.endTime.tv_usec) -
		(benchmarker.startTime.tv_sec * (uint64_t)1000000 +
		 benchmarker.startTime.tv_usec));
}

uint64_t getElapsedTimeMs(bench *benchParam)
{
	if (!benchParam) {
		return (0);
	}
	bench benchmarker = *benchParam;
	return (((benchmarker.endTime.tv_sec * (uint64_t)1000000 +
		  benchmarker.endTime.tv_usec) -
		 (benchmarker.startTime.tv_sec * (uint64_t)1000000 +
		  benchmarker.startTime.tv_usec)) /
		1000);
}

uint64_t getElapsedTimeS(bench *benchParam)
{
	if (!benchParam) {
		return (0);
	}
	bench benchmarker = *benchParam;
	return (((benchmarker.endTime.tv_sec * (uint64_t)1000000 +
		  benchmarker.endTime.tv_usec) -
		 (benchmarker.startTime.tv_sec * (uint64_t)1000000 +
		  benchmarker.startTime.tv_usec)) /
		1000000);
}

void freeTimer(bench **timer)
{
	bfree(*timer);
	*timer = NULL;
}
