#define _GNU_SOURCE

#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define NSEC_IN_SEC 1000000000 // 1,000,000,000
#define PAGESIZE 4096 // 4kB page size

struct timespec typedef timespec_t;

static uint64_t elapsed_nsecs(timespec_t *start, timespec_t *end) {
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

int main(int argc, char **argv) {

  if (argc != 4) {
    fprintf(stderr, "Invalid arguments: ./tlb npages, print_clock, print_cpu\n");
    exit(EXIT_FAILURE);
  }

  uint32_t NUMPAGES = strtol(argv[1], NULL, 10);
  uint32_t print_clock = strtol(argv[2], NULL, 10);
  uint32_t print_cpu = strtol(argv[3], NULL, 10);

  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);

  if ((sched_setaffinity(getpid(), sizeof(cpu_set_t), &set)) != 0) {
    fprintf(stderr, "Error setting CPU affinity. %i: %s", errno, strerror(errno));
  }

  if (print_cpu) {
    fprintf(stdout, "PID: %i. CPU: %i\n. CPU: %i\n", 
      getpid(), sched_getcpu(), CPU_ISSET(0, &set)
    );
  }

  // get system clock resolution
  struct timespec tp = { 0 };
  if (clock_getres(CLOCK_MONOTONIC_RAW, &tp) < 0) {
    fprintf(stderr, "Timer not supported\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  // print on test runs
  if (print_clock) {
    fprintf(stdout, "System clock resolution: %jd:%09ld\n", (intmax_t) tp.tv_sec, tp.tv_nsec);
  }

  // allocate array in PAGESIZE blocks * NUMPAGES length
  uint32_t len = (PAGESIZE / sizeof(uint32_t)) * NUMPAGES;
  uint32_t *arr = NULL;
  if ((arr = calloc(sizeof(uint32_t), len)) == NULL) {
      fprintf(stderr, "Error allocating memory. %i: %s", errno, strerror(errno));
      exit(EXIT_FAILURE);
  }

  // allocate array to log elapsed time to calculate an avg
  uint64_t *log = NULL;
  if ((log = calloc(sizeof(uint64_t), NUMPAGES)) == NULL) {
      fprintf(stderr, "Error allocating memory. %i: %s", errno, strerror(errno));
      exit(EXIT_FAILURE);
  }

  timespec_t start = { 0 }, end = { 0 };

  uint32_t jump = PAGESIZE / sizeof(uint64_t);
  uint32_t idx = 0;

  for (uint64_t i = 0; i < NUMPAGES * jump; i += jump) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    arr[i] += 1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    // log the time to access the current page
    log[idx++] += elapsed_nsecs(&start, &end);
  }

  // print elapsed page access times
  for (uint32_t i = 0; i < NUMPAGES; i++) {
    fprintf(stdout, "%llu\n", log[i]);
  }

}
