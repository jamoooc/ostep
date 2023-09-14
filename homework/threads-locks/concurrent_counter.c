/*
  Build a simple concurrent counter and measure how long it
  takes to increment the counter many times as the number of threads
  increases. How many CPUs are available on the system you are
  using? Does this number impact your measurements at all?
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_COUNT 1000
#define MAX_COUNT 1000000 // 1,000,000
#define NSEC_IN_SEC 1000000000 // 1,000,000,000

typedef struct counter_t {
  int value;
  pthread_mutex_t lock;
} counter_t;

typedef struct args_t {
  int thread_id;
  counter_t *c;
} args_t;

typedef struct timespec timespec_t;

void init_counter(counter_t *c) {
  c->value = 0;
  if ((pthread_mutex_init(&c->lock, NULL)) > 0) {
    fprintf(stderr, "Error initialising mutex\n");
    exit(EXIT_FAILURE);
  }
}

void increment_counter(counter_t *c) {
  if ((pthread_mutex_lock(&c->lock)) > 0) {
    fprintf(stderr, "Error getting mutex\n");
    exit(EXIT_FAILURE);
  }
  c->value++;
  if ((pthread_mutex_unlock(&c->lock)) > 0) {
    fprintf(stderr, "Error releasing mutex\n");
    exit(EXIT_FAILURE);
  }
}

void decrement_counter(counter_t *c) {
  if ((pthread_mutex_lock(&c->lock)) > 0) {
    fprintf(stderr, "Error getting mutex\n");
    exit(EXIT_FAILURE);
  }
  c->value--;
  if ((pthread_mutex_unlock(&c->lock)) > 0) {
    fprintf(stderr, "Error releasing mutex\n");
    exit(EXIT_FAILURE);
  }
}

int get_count(counter_t *c) {
  if ((pthread_mutex_lock(&c->lock)) > 0) {
    fprintf(stderr, "Error getting mutex\n");
    exit(EXIT_FAILURE);
  }
  int counter = c->value;
  if ((pthread_mutex_unlock(&c->lock)) > 0) {
    fprintf(stderr, "Error getting mutex\n");
    exit(EXIT_FAILURE);
  }
  return counter;
}

void *start_routine(void *args) {
  args_t *a = (args_t *) args;
  counter_t *c = (counter_t *) a->c;
  for (int i = 0; i < (MAX_COUNT / THREAD_COUNT); i++) {
    increment_counter((counter_t *) c);
  }
  return NULL;
}

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

int main(void) {

  counter_t c;
  init_counter(&c);

  args_t args[THREAD_COUNT];
  pthread_t threads[THREAD_COUNT];

  timespec_t t1, t2;

  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

  for (int i = 0; i < THREAD_COUNT; i++) {
    args[i].c = &c;
    args[i].thread_id = i;
    pthread_create(&threads[i], NULL, start_routine, &args[i]);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
  fprintf(stdout, "Elapsed time: %llu\n", elapsed_nsecs(&t1, &t2));

  return EXIT_SUCCESS;
}
