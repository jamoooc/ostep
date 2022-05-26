// Measuring the cost of a context switch is a little trickier. The lmbench
// benchmark does so by running two processes on a single CPU, and set-
// ting up two UNIX pipes between them; a pipe is just one of many ways
// processes in a UNIX system can communicate with one another. The first
// process then issues a write to the first pipe, and waits for a read on the
// second; upon seeing the first process waiting for something to read from
// the second pipe, the OS puts the first process in the blocked state, and
// switches to the other process, which reads from the first pipe and then
// writes to the second. When the second process tries to read from the first
// pipe again, it blocks, and thus the back-and-forth cycle of communication
// continues. By measuring the cost of communicating like this repeatedly,
// lmbench can make a good estimate of the cost of a context switch. You
// can try to re-create something similar here, using pipes, or perhaps some
// other communication mechanism such as UNIX sockets.

// measure time cost of context switch

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_WRITE 100
#define NSEC_IN_SEC 1000000000 // 1,000,000,000

typedef struct timespec timespec_t;

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

static uint64_t average_cost(uint64_t *times) {
  uint64_t tmp = 0;
  for (int i = 0; i < MAX_WRITE * 2; i++) {
    tmp += times[i];
  }
  return tmp / MAX_WRITE * 2;
}

enum {
  READ,
  WRITE
};

enum {
  CHILD1,
  CHILD2,
  DATA // this pipe allows each process to write to a shared array
};

int main(void) {

  uint64_t avg = 0;
  timespec_t start = { 0 }, end = { 0 };
  uint64_t cs_costs[MAX_WRITE * 2] = { 0 };

  // TODO: sched_setaffinity

  // create two unidirectional pipes, by default they will be
  // blocking and allow the CPU to perform a context switch
  int fildes[3][2];
  if (pipe(fildes[CHILD1]) < 0 || pipe(fildes[CHILD2]) < 0 || pipe(fildes[DATA]) < 0) {
    fprintf(stderr, "Error creating pipe. %i: %s.\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  pid_t pid1 = fork();
  if (pid1 < 0) {
    fprintf(stderr, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid1 == 0) {
    fprintf(stdout, "Process one started. PID: %i.\n", getpid());
    char c;
    for (int i = 0; i < MAX_WRITE; i++) {

      if (write(fildes[CHILD2][WRITE], "b", sizeof("b") - 1) != 1) {
        fprintf(stderr, "Error writing to pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      int rv = 0;
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);
      rv = read(fildes[CHILD1][READ], &c, sizeof("a") - 1); // blocking read
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);

      if (rv < 1) {
        fprintf(stderr, "Error reading from pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      // write elapsed nsecs to the shared data pipe
      uint64_t diff = elapsed_nsecs(&start, &end); 
      if (write(fildes[DATA][WRITE], &diff, sizeof(diff)) != sizeof(diff)) {
        fprintf(stderr, "Error writing to data pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    fprintf(stdout, "Exiting process one.\n");
    exit(EXIT_SUCCESS);
  }

  pid_t pid2 = fork();
  if (pid2 < 0) {
    fprintf(stderr, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid2 == 0) {
    fprintf(stdout, "Process two started. PID: %i.\n", getpid());

    char c;
    for (int i = 0; i < MAX_WRITE; i++) {
      if (write(fildes[CHILD1][WRITE], "a", sizeof("a") - 1) != 1) {
        fprintf(stderr, "Error writing to pipe in process two. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
      int rv = 0;
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);
      rv = read(fildes[CHILD2][READ], &c, sizeof("b") - 1);
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);
      if (rv < 1) {
        fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      // write elapsed nsecs to the shared data pipe
      uint64_t diff = elapsed_nsecs(&start, &end); 
      if (write(fildes[DATA][WRITE], &diff, sizeof(diff)) != sizeof(diff)) {
        fprintf(stderr, "Error writing to data pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    fprintf(stdout, "Exiting process two.\n");
    exit(EXIT_SUCCESS);
  }

  waitpid(pid1, 0, 0);
  waitpid(pid2, 0, 0);

  // read the elapsed times from the data pipe and print the average
  for (int i = 0; i < 200; i++) {
    if (read(fildes[DATA][READ], &cs_costs[i], sizeof(uint64_t)) != sizeof(uint64_t)) {
      fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  fprintf(stdout, "Average context switch cost: %llu.\n", average_cost(cs_costs));

  close(fildes[CHILD1][READ]);
  close(fildes[CHILD1][WRITE]);
  close(fildes[CHILD2][READ]);
  close(fildes[CHILD2][WRITE]);
  close(fildes[DATA][READ]);
  close(fildes[DATA][WRITE]);

  exit(EXIT_SUCCESS);
}







  // if (fork() == 0) {

  //   char c;
  //   for (int i = 0; i < 5; i++) {

  //     fprintf(stdout, "child1 write\n");
  //     write(fildes[CHILD1][WRITE], "a", sizeof("a") - 1);
  //     fprintf(stdout, "child1 waiting to read\n");
  //     read(fildes[CHILD2][READ], &c, sizeof("b") - 1);
  //     fprintf(stdout, "child1 read: %c\n", c);

  //   }


  // } else if (fork() == 0) {
    
  //   char c;
  //   for (int i = 0; i < 5; i++) {

  //     fprintf(stdout, "child2 write\n");
  //     write(fildes[CHILD2][WRITE], "b", sizeof("b") - 1);
  //     fprintf(stdout, "child2 waiting to read\n");
  //     read(fildes[CHILD1][READ], &c, sizeof("a") - 1);
  //     fprintf(stdout, "child2 read: %c\n", c);

  //   }

  // } else {
  //   wait(NULL);
  //   wait(NULL);
  // }


  // pid_t pid1 = fork();
  // if (pid1 < 0) {
  //   fprintf(stdout, "Error forking process.\n");
  //   exit(EXIT_FAILURE);
  // } else if (pid1 == 0) {
  //   fprintf(stdout, "Child1 PID: %i\n", getpid());

  //   pid_t pid2 = fork();
  //   if (pid2 < 0) {
  //     fprintf(stdout, "Error forking process.\n");
  //     exit(EXIT_FAILURE);
  //   } else if (pid2 == 0) {
  //     fprintf(stdout, "Child2 PID: %i\n", getpid());

  //     char c;
  //     for (int i = 0; i < 5; i++) {
  //       fprintf(stdout, "child1 write\n");
  //       write(fildes[CHILD1][WRITE], "a", sizeof("a") - 1);
  //       fprintf(stdout, "child1 waiting to read\n");
  //       read(fildes[CHILD2][READ], &c, sizeof("b") - 1);
  //       fprintf(stdout, "child1 read: %c\n", c);
  //     }
  //     exit(EXIT_SUCCESS);
  //   } else {

  //     char c;
  //     for (int i = 0; i < 5; i++) {
  //       fprintf(stdout, "child2 write\n");
  //       write(fildes[CHILD2][WRITE], "b", sizeof("b") - 1);
  //       fprintf(stdout, "child2 waiting to read\n");
  //       read(fildes[CHILD1][READ], &c, sizeof("a") - 1);
  //       fprintf(stdout, "child2 read: %c\n", c);
  //     }

  //     fprintf(stdout, "Waiting for child 2.\n");
  //     waitpid(pid2, 0, 0);
  //     exit(EXIT_SUCCESS);
  //   }
  // } 
  // fprintf(stdout, "Waiting for child 1.\n");
  // waitpid(pid1, 0, 0);
  // fprintf(stdout, "Done.\n");
  // exit(EXIT_SUCCESS);

