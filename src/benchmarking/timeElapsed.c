#include "timeElapsed.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

bench *startTimer(void) {
  errno = 0;
  bench *benchmarker = malloc(sizeof(struct timeObj));
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

void endTimer(bench *benchmarker) {
  if (!benchmarker) {
    return;
  }
  gettimeofday(&(benchmarker->endTime), NULL);
}

uint64_t getElapsedTimeUs(bench *benchParam) {
  if (!benchParam) {
    return (0);
  }
  return ((benchParam->endTime.tv_sec * (uint64_t)1000000 +
           benchParam->endTime.tv_usec) -
          (benchParam->startTime.tv_sec * (uint64_t)1000000 +
           benchParam->startTime.tv_usec));
}

void getElapsedTimeUs_print(bench *benchParam) {
  if (!benchParam) {
    return;
  }
  printf("Took %ld Us\n",
         (benchParam->endTime.tv_sec * (uint64_t)1000000 +
          benchParam->endTime.tv_usec) -
             (benchParam->startTime.tv_sec * (uint64_t)1000000 +
              benchParam->startTime.tv_usec));
}

uint64_t getElapsedTimeMs(bench *benchParam) {
  if (!benchParam) {
    return (0);
  }
  return (((benchParam->endTime.tv_sec * (uint64_t)1000000 +
            benchParam->endTime.tv_usec) -
           (benchParam->startTime.tv_sec * (uint64_t)1000000 +
            benchParam->startTime.tv_usec)) /
          1000);
}

void getElapsedTimeMs_print(bench *benchParam) {
  if (!benchParam) {
    return;
  }
  printf("Took %ld Ms\n", (((benchParam->endTime.tv_sec * (uint64_t)1000000 +
                             benchParam->endTime.tv_usec) -
                            (benchParam->startTime.tv_sec * (uint64_t)1000000 +
                             benchParam->startTime.tv_usec)) /
                           1000));
}

uint64_t getElapsedTimeS(bench *benchParam) {
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

void getElapsedTimeS_print(bench *benchParam) {
  if (!benchParam) {
    return;
  }
  bench benchmarker = *benchParam;
  printf("Took %ld S\n", (((benchmarker.endTime.tv_sec * (uint64_t)1000000 +
                            benchmarker.endTime.tv_usec) -
                           (benchmarker.startTime.tv_sec * (uint64_t)1000000 +
                            benchmarker.startTime.tv_usec)) /
                          1000000));
}

uint64_t getElapsedTimeNs(bench *benchParam) {
  if (!benchParam) {
    return (0);
  }
  return ((benchParam->endTime.tv_sec * (uint64_t)1E+9 +
           (uint64_t)(benchParam->endTime.tv_usec * 1000) -
           (benchParam->startTime.tv_sec * (uint64_t)1E+9 +
            (uint64_t)(benchParam->startTime.tv_usec * 1000))));
}

void getElapsedTimeNs_print(bench *benchParam) {
  if (!benchParam) {
    return;
  }
  printf("Took %ld Ns\n",
         ((benchParam->endTime.tv_sec * (uint64_t)1E+9 +
           (uint64_t)(benchParam->endTime.tv_usec * 1000) -
           (benchParam->startTime.tv_sec * (uint64_t)1E+9 +
            (uint64_t)(benchParam->startTime.tv_usec * 1000)))));
}

void freeTimer(bench **timer) {
  free(*timer);
  *timer = NULL;
}
