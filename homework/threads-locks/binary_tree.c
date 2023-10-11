/*
  Pick your favorite data structure, such as a B-tree or other slightly
  more interesting structure. Implement it, and start with a simple
  locking strategy such as a single lock. Measure its performance as
  the number of concurrent threads increases.

  gcc -o bin/binary_tree binary_tree.c timer.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

typedef struct btree_node_t {
  int value;
  pthread_mutex_t node_lock;
  struct btree_node_t *left;
  struct btree_node_t *right;
} btree_node_t;

typedef struct btree_root_t {
  btree_node_t *root;
  pthread_mutex_t root_lock;
} btree_root_t;

typedef struct args_t {
  btree_root_t *btree;
  int iter;
  pthread_mutex_t iter_lock;
  uint64_t *search_times;
} args_t;

#define THREAD_COUNT 128
#define NODE_COUNT 1000000

static btree_node_t *create_node(int value) {
  btree_node_t *node = NULL;
  if ((node = malloc(sizeof(btree_node_t))) == NULL) {
    fprintf(stderr, "Error allocating memory");
    exit(EXIT_FAILURE);
  }
  node->value = value;
  node->left = NULL;
  node->right = NULL;
  return node;
}

static void init_btree(btree_root_t *btree, int value) {
  btree_node_t *node = NULL;
  node = create_node(value);
  btree->root = node;
  pthread_mutex_init(&btree->root_lock, NULL);
}

static void insert_node(btree_node_t *node, int value) {
  if (node->value < value) {
    if (node->left == NULL) {
      node->left = create_node(value);
    } else {
      insert_node(node->left, value);
    }
  } else {
    if (node->right == NULL) {
      node->right = create_node(value);
    } else {
      insert_node(node->right, value);
    }
  }
}

static int contains(btree_node_t *node, int value) {
  if (node->value == value) {
    return 1;
  } else if (node->value < value) {
    if (node->left == NULL) {
      return 0;
    } else {
      return contains(node->left, value);
    }
  } else {
    if (node->right == NULL) {
      return 0;
    } else {
      return contains(node->right, value);
    }
  }
}

static void print_in_order(btree_node_t *node) {
  if (node->left != NULL) {
    print_in_order(node->left);
  }
  fprintf(stdout, "%i\n", node->value);
  if (node->right != NULL) {
    print_in_order(node->right);
  }
}

void *single_lock_contains(void *args) {
  args_t *a = (args_t *) args;
  timespec_t t1, t2;
  
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  
  pthread_mutex_lock(&a->btree->root_lock);
  contains(a->btree->root, NODE_COUNT - 1);
  pthread_mutex_unlock(&a->btree->root_lock);
  
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
  
  pthread_mutex_lock(&a->iter_lock);
  a->search_times[a->iter++] = elapsed_nsecs(&t1, &t2);
  pthread_mutex_unlock(&a->iter_lock);
  
  return NULL;
}

int main(void) {
  btree_root_t btree;
  int initial_value = 0;
  init_btree(&btree, initial_value);

  timespec_t t1, t2;
  pthread_t threads[THREAD_COUNT];
  uint64_t search_times[THREAD_COUNT] = { 0 };

  args_t args = { 0 };
  args.btree = &btree;
  args.iter = 0;
  pthread_mutex_init(&args.iter_lock, NULL);
  args.search_times = search_times;

  for (int i = 0; i < NODE_COUNT; i++) {
    insert_node(btree.root, arc4random_uniform(NODE_COUNT));
  } 

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&threads[i], NULL, single_lock_contains, &args);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }
  
  // print_in_order(btree.root);
  fprintf(stdout, "Average time to find greatest node: %lluns\n", average_cost(search_times, THREAD_COUNT));
}
