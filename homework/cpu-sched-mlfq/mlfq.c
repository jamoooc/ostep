// MLFQ simulation

// Process nodes are run in round robin and demoted to a lower
// priority queue after their cpu allotment. All process are 
// intermittently raised to the highest priority queue.

// gcc -o bin/mlfq mlfq.c

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PROC 6 // number of process to seed top level process queue
#define NUM_QUEUE 5 // number fo process queues
#define CPU_ALLOT 2 // maximum time spend at a priority level
#define TIME_MODULO 10 // modulo a random number to get remaining_time for each process
#define RESET_PRIORITY 4 // reset priority of all processes after n iterations

typedef struct process {
  uint32_t pid;
  uint32_t cpu_time; // cpu time elapsed
  int64_t remaining_time; // cpu time remaining
  struct process *next;
  struct process *prev;
} process_t;

static void handle_error(char *err);
static process_t **init_ll(process_t **process_queue);
static void init_queues(process_t **process_queues[]);
static void print_ll(process_t **process_queue);
static void print_queues(process_t **process_queues[]);
static int32_t jobs(process_t **process_queues[]);
static void round_robin(int32_t queue_level, const int64_t queue_quanta[], process_t **process_queues[]);
static void remove_node(process_t *node, process_t **process_queue);
static void demote_process(process_t *node, process_t **cur_queue, process_t **process_queue);
static void reset_process_priority(process_t **process_queue[]);

int main(void)
{
  // create an array of ptrs to ptrs to process_ts and initialise the top level queue.
  // each is the head of a linked list representing a process queue 
  process_t **process_queues[NUM_QUEUE] = { 0 };
  init_queues(&process_queues[0]);
  process_queues[0] = init_ll(process_queues[0]);
  print_queues(process_queues);

  const int64_t queue_quanta[] = { 1, 2, 3 };

  int32_t count = 0;
  int32_t queue_level = -1;
  while((queue_level = jobs(process_queues)) >= 0) {
    if (count++ == RESET_PRIORITY) {
      count = 0;
      reset_process_priority(process_queues);
    }
    printf("Processing queue level: %i\n", queue_level);
    round_robin(queue_level, queue_quanta, process_queues);
    sleep(1);
  }

  printf("All processes complete\n");
  free(process_queues[0]);
  free(process_queues[1]);
  free(process_queues[2]);
  exit(EXIT_SUCCESS);
}

static void round_robin(int32_t queue_level, const int64_t queue_quanta[], process_t **process_queues[])
{
  process_t *cur = *process_queues[queue_level];

  while (cur != NULL) {
    print_queues(process_queues);

    // decrement remaining time and increment cpu time at current level
    printf("\nPID%i: remaining_time: %lli", cur->pid, cur->remaining_time);
    cur->remaining_time -= queue_quanta[queue_level];
    printf("->%lli", cur->remaining_time);

    printf(" cpu_time: %i", cur->cpu_time);
    cur->cpu_time++;
    printf("->%i\n\n", cur->cpu_time);

    process_t *next_node = cur->next;

    // remove complete process
    if (cur->remaining_time <= 0) {
      remove_node(cur, process_queues[queue_level]);
      cur = next_node;
      sleep(1);
      continue;
    }

    // demote process after cpu allotment
    if (cur->cpu_time >= CPU_ALLOT) {
      if (queue_level < NUM_QUEUE - 1) {
        demote_process(cur, process_queues[queue_level], process_queues[queue_level + 1]);
        cur = next_node;
        sleep(1);
        continue;
      }
    }
    cur = next_node;
    sleep(1);
  }
}

static void reset_process_priority(process_t **process_queue[])
{
  printf("Resetting priority of all queues.\n");
  process_t **top = process_queue[0];
    process_t **tmp = NULL;

  for (int i = 1; i < NUM_QUEUE - 1; i++) {
    tmp = process_queue[i];
    while (*top != NULL) {
      top = &(*top)->next;
    }
    *top = *tmp;
    *tmp = NULL;
  }
}

static void demote_process(process_t *node, process_t **cur_queue, process_t **next_queue)
{
  if (*cur_queue == NULL || node == NULL) {
    return;
  }
  printf("PID: %i CPU allotment complete. Demoting node.\n", node->pid);

  // reset cpu timer
  node->cpu_time = 0;

  if (*cur_queue == node) {
    *cur_queue = node->next;
  }

  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  if (node->prev != NULL) {
    node->prev->next = node->next;
  }

  if (*next_queue != NULL) {
    (*next_queue)->prev = node;
  }
  node->next = *next_queue;
  *next_queue = node;
}

// return queue number with waiting jobs, or falsey value
static int32_t jobs(process_t **process_queues[])
{
  for (uint32_t i = 0; i < NUM_QUEUE; i++) {
    process_t **tmp = process_queues[i];
    if (tmp == NULL) {
      continue; // there is no head of the queue
    }
    if (*tmp != NULL) {
      return i;
    }
  }
  return -1;
}

// return queue number with waiting jobs, or falsey value
static void print_queues(process_t **process_queues[])
{
  puts("");
  for (int i = 0; i < NUM_QUEUE; i++) {
    process_t **tmp = process_queues[i];
    printf("%i: ", i);
    if (tmp != NULL && *tmp != NULL) {

      print_ll(tmp);
    } else {
      puts("NULL");
    }
  }
}

static void handle_error(char *err)
{
  fprintf(stderr, "%s. %i: %s\n", 
    err, errno, strerror(errno)
  );
  exit(EXIT_FAILURE);
}

static void init_queues(process_t **process_queues[])
{
  for (int i = 0; i < NUM_QUEUE; i++) {
    process_queues[i] = calloc(1, sizeof(process_t **));
  }
}

static process_t **init_ll(process_t **head)
{
  // process_t **head = calloc(1, sizeof(process_t **));
  process_t *tmp = NULL;
  for (int i = 0; i < NUM_PROC; i++) {
    tmp = calloc(1, sizeof(process_t));
    tmp->pid = i + 1;
    tmp->cpu_time = 0;
    tmp->remaining_time =  (random() + 1) % TIME_MODULO;
    tmp->prev = NULL;
    tmp->next = *head;
    if (*head != NULL) {
      (*head)->prev = tmp;
    }
    *head = tmp;
  }
  return head;
}

static void print_ll(process_t **process_queue)
{
  process_t *tmp = NULL;
  tmp = *process_queue;
  while (tmp != NULL) {
    printf("[PID: %i] -> ", tmp->pid);
    tmp = tmp->next;
  }
  puts("NULL");
}

static void remove_node(process_t *node, process_t **process_queue)
{
  printf("PID:%i complete. Removing node.\n", node->pid);
  if (*process_queue == NULL || node == NULL) {
    return;
  }

  if (*process_queue == node) {
    *process_queue = node->next;
  }

  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  if (node->prev != NULL) {
    node->prev->next = node->next;
  }

  free(node);
}
