#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>
#include <stdlib.h>

typedef struct timespec timespec_t;

uint64_t elapsed_nsecs(timespec_t *start, timespec_t *end);
uint64_t average_cost(uint64_t *costs, uint32_t count);

#endif
