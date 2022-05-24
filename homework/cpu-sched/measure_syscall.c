// In this homework, you’ll measure the costs of a system call and context
// switch. Measuring the cost of a system call is relatively easy. For example,
// you could repeatedly call a simple system call (e.g., performing a 0-byte
// read), and time how long it takes; dividing the time by the number of
// iterations gives you an estimate of the cost of a system call.

// measure time cost of a system call

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_SYSCALL 100
#define NSEC_IN_SEC 1000000000 // 1,000,000,000
#define TMP_FILE "tmp.txt"

typedef struct timespec timespec_t;

timespec_t diff(timespec_t start, timespec_t end);
uint64_t average_cost(timespec_t *times);

int main(void) {

  uint64_t avg = 0;
  timespec_t start = { 0 }, end = { 0 };
  timespec_t clock_res = { 0 };
  timespec_t tmp = { 0 };
  timespec_t times[MAX_SYSCALL] = { 0 };

  if ((clock_getres(CLOCK_PROCESS_CPUTIME_ID, &clock_res)) < 0) {
    fprintf(stderr, "Error getting clock resolution. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  };
  printf("Minimum system clock resolution: %ld nsecs\n", clock_res.tv_nsec);

  FILE *fp = fopen(TMP_FILE, "w");

  for (int i = 0; i < MAX_SYSCALL; i++) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    fwrite("a", sizeof("a"), 1, fp);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    times[i] = diff(start, end);
  }
  fprintf(stdout, "Average system call time: %llu nsecs\n", average_cost(times));

  if (fclose(fp) == EOF) {
    fprintf(stderr, "Error closing file. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  };

  if (unlink(TMP_FILE) < 0) {
    fprintf(stderr, "Error removing tmp file. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

timespec_t diff(timespec_t start, timespec_t end) {
  timespec_t temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = NSEC_IN_SEC + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}

uint64_t average_cost(timespec_t *times) {
  uint64_t tmp = 0;
  for (int i = 0; i < MAX_SYSCALL; i++) {
    tmp += times[i].tv_nsec;
  }
  return tmp / MAX_SYSCALL;
}
