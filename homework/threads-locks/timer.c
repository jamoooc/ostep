#include <time.h>
#include "timer.h"

#define NSEC_IN_SEC 1000000000 // 1,000,000,000

uint64_t elapsed_nsecs(timespec_t *start, timespec_t *end) {
  timespec_t temp = { 0 };
  // its possible for (end->nsec - start->nsec) to be negative, if so subract 1 full second
  if ((end->tv_nsec - start->tv_nsec) < 0) {
    temp.tv_sec = end->tv_sec - start->tv_sec - 1;
    temp.tv_nsec = NSEC_IN_SEC + end->tv_nsec - start->tv_nsec; // add value of negative nsec
  } else {
    temp.tv_sec = end->tv_sec - start->tv_sec;
    temp.tv_nsec = end->tv_nsec - start->tv_nsec;
  }
  return (temp.tv_sec * NSEC_IN_SEC) + temp.tv_nsec;
}

uint64_t average_cost(uint64_t *costs, uint32_t count) {
  uint64_t tmp = 0;
  for (uint32_t i = 0; i < count; i++) {
    tmp += costs[i];
  }
  return tmp / count;
}
