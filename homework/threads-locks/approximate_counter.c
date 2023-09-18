/*
  Next, build a version of the sloppy counter. Once again, measure its
  performance as the number of threads varies, as well as the thresh-
  old. Do the numbers match what you see in the chapter?
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_COUNT 4
#define SAMPLE_COUNT 10
#define MAX_COUNT 4000000
#define THRESHOLD 1
#define NSEC_IN_SEC 1000000000 // 1,000,000,000

typedef struct counter_t {
  int global_count;
  pthread_mutex_t global_lock;
  int local_thread[THREAD_COUNT];
  pthread_mutex_t local_thread_lock[THREAD_COUNT];
  int threshold;
} counter_t;

typedef struct args_t {
  int thread_id;
  counter_t *c;
} args_t;

typedef struct timespec timespec_t;

void init_counter(counter_t *c, int threshold) {
  c->threshold = threshold;
  c->global_count = 0;
  pthread_mutex_init(&c->global_lock, NULL);
  for (int i = 0; i < THREAD_COUNT; i++) {
    c->local_thread[i] = 0;
    pthread_mutex_init(&c->local_thread_lock[i], NULL);
  }
}

void update_counter(counter_t *c, int thread_id, int amount) {
  int cpu = thread_id % THREAD_COUNT;
  pthread_mutex_lock(&c->local_thread_lock[cpu]);
  c->local_thread[cpu] += amount;
  if (c->local_thread[cpu] >= c->threshold) {
    pthread_mutex_lock(&c->global_lock);
    c->global_count += c->local_thread[cpu];
    pthread_mutex_unlock(&c->global_lock);
    c->local_thread[cpu] = 0;
  }
  pthread_mutex_unlock(&c->local_thread_lock[cpu]);
}

// approximate global count (within threshold * THREAD_COUNT)
int get_global_count(counter_t *c) {
  pthread_mutex_lock(&c->global_lock);
  int global_count = c->global_count;
  pthread_mutex_unlock(&c->global_lock);
  return global_count;
}

void *start_routine(void *args) {
  args_t *a = (args_t *) args;
  counter_t *c = (counter_t *) a->c;
  int thread_id = a->thread_id;
  for (int i = 0; i < MAX_COUNT; i++) {
    update_counter(c, thread_id, 1);
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

int main() {
  counter_t c;
  init_counter(&c, THRESHOLD);

  args_t args[THREAD_COUNT];
  pthread_t threads[THREAD_COUNT];

  timespec_t t1, t2;
  uint64_t samples[SAMPLE_COUNT] = { 0 };

  for (int sample = 0; sample < SAMPLE_COUNT; sample++) {
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
    samples[sample] = elapsed_nsecs(&t1, &t2);
  }
  
  uint64_t sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += samples[i];
  }

  fprintf(stdout, "Elapsed time: %llu\n", sum / SAMPLE_COUNT);

  return EXIT_SUCCESS;
}
