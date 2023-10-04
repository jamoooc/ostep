/*
  Build a version of a linked list that uses hand-over-hand locking
  [MS04], as cited in the chapter. You should read the paper first
  to understand how it works, and then implement it. Measure its
  performance. When does a hand-over-hand list work better than a
  standard list as shown in the chapter?
  
  gcc -o bin/hoh_linked_list_threads hoh_linked_list_threads.c timer.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

typedef struct node_t {
  int key;
  struct node_t *next;
  pthread_mutex_t lock;
} node_t;

typedef struct list_t {
  node_t *head;
  pthread_mutex_t lock;
} list_t;

typedef struct args_t {
  list_t *list;
  int iter;
  pthread_mutex_t iter_lock;
  uint64_t *search_times;
} args_t;

#define NODE_COUNT 4000000
#define THREAD_COUNT 64

void list_init(list_t *list) {
  list->head = NULL;
  pthread_mutex_init(&list->lock, NULL);
}

void *mutex_lock(pthread_mutex_t *lock, void *(*cb)(pthread_mutex_t *), pthread_mutex_t *args) {
  if ((pthread_mutex_lock(lock)) != 0) {
    fprintf(stderr, "Error locking mutex.\n");
    if (cb) {
      printf("CB!\n");
      cb(args);
    } else {
      exit(EXIT_FAILURE);
    }
  }
  return NULL;
}

void *mutex_unlock(pthread_mutex_t *lock) {
  if ((pthread_mutex_unlock(lock)) != 0) {
    fprintf(stderr, "Error unlocking mutex.\n");
    exit(EXIT_FAILURE);
  }
  return NULL;
}

int prepend_node(list_t *list, int key) {
  node_t *node = NULL;
  if ((node = malloc(sizeof(node_t))) == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(EXIT_FAILURE);
  }
  node->key = key;
  pthread_mutex_init(&node->lock, NULL);
  mutex_lock(&list->lock, NULL, NULL);
  mutex_lock(&node->lock, NULL, NULL);
  node->next = list->head;
  list->head = node;
  mutex_unlock(&node->lock);
  mutex_unlock(&list->lock);
  return 0;
}

int hoh_lookup_node(list_t *list, int key) {
  int rv = -1;
  node_t *curr = list->head;
  mutex_lock(&list->lock, NULL, NULL); 
  while (curr) {
    if (curr->key == key) {
      rv = 0;
      mutex_unlock(&curr->lock);
      break;
    }
    if (curr->next) {
      mutex_lock(&curr->next->lock, mutex_unlock, &list->lock);
    }
    mutex_unlock(&curr->lock);
    curr = curr->next;
  }
  mutex_unlock(&list->lock);
  return rv;
}

int lookup_node(list_t *list, int key) {
  int rv = -1;
  node_t *curr = list->head;
  pthread_mutex_lock(&list->lock);
  while (curr) {
    if (curr->key == key) {
      rv = 0;
      break;
    }
    curr = curr->next;
  }
  pthread_mutex_unlock(&list->lock);
  return rv;
}

void *hoh_start_routine(void *args) {
  args_t *a = (args_t *) args;
  timespec_t t1, t2;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  hoh_lookup_node(a->list, NODE_COUNT - 1);
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
  a->search_times[a->iter++] = elapsed_nsecs(&t1, &t2);
  return NULL;
}

void *single_lock_start_routine(void *args) {
  args_t *a = (args_t *) args;
  timespec_t t1, t2;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  lookup_node(a->list, NODE_COUNT - 1);
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
  a->search_times[a->iter++] = elapsed_nsecs(&t1, &t2);
  return NULL;
}

int main(void) {
  list_t list;
  list_init(&list);
  
  timespec_t t1, t2;
  pthread_t threads[THREAD_COUNT];

  uint64_t search_times[THREAD_COUNT] = { 0 };

  args_t args = { 0 };
  args.iter = 0;
  pthread_mutex_init(&args.iter_lock, NULL);
  args.list = &list;
  args.search_times = search_times;

  for (int i = 0; i < NODE_COUNT; i++) {
    prepend_node(&list, i);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&threads[i], NULL, hoh_start_routine, &args);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }
  
  fprintf(stdout, "Average time to find last node with HOH linked list: %lluns\n", average_cost(search_times, THREAD_COUNT));
  
  args.iter = 0;

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&threads[i], NULL, single_lock_start_routine, &args);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }
  
  fprintf(stdout, "Average time to find last node single locked linked list: %lluns\n", average_cost(search_times, THREAD_COUNT));
}
