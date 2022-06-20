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
#define _GNU_SOURCE 

#include <time.h>
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_WRITE 100
#define NSEC_IN_SEC 1000000000 // 1,000,000,000

typedef struct timespec timespec_t;
// typedef u_int64_t uint64_t;

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
  DATA1, // this pipe allows each process to write to a shared array
  DATA2
};

int main(void) {

  cpu_set_t set;
  uint64_t avg = 0;
  timespec_t clock_res = { 0 };
  timespec_t start1 = { 0 }, end1 = { 0 };
  timespec_t start2 = { 0 }, end2 = { 0 };

  // create two unidirectional pipes for the child processes communication, 
  // and two data pipes to receive timespec_t data from each process. By 
  // default the pipes block and allow the CPU to perform a context switch.
  int fildes[4][2];
  if (pipe(fildes[CHILD1]) < 0 || pipe(fildes[CHILD2]) < 0) {
    fprintf(stderr, "Error creating communication pipes. %i: %s.\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (pipe(fildes[DATA1]) < 0 || pipe(fildes[DATA2]) < 0) {
    fprintf(stderr, "Error creating data pipes. %i: %s.\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if ((clock_getres(CLOCK_MONOTONIC_RAW, &clock_res)) < 0) {
    fprintf(stderr, "Error getting clock resolution. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  };
  printf("Minimum system clock resolution: %ld nsecs\n", clock_res.tv_nsec);

  pid_t pid1 = fork();
  if (pid1 < 0) {
    fprintf(stderr, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid1 == 0) {

    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(cpu_set_t), &set);
    fprintf(stdout, "Process one started. PID: %i. CPU: %i\n", getpid(), sched_getcpu());

    char c;
    for (int i = 0; i < MAX_WRITE; i++) {

      if (write(fildes[CHILD2][WRITE], "b", sizeof("b") - 1) != 1) {
        fprintf(stderr, "Error writing to pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      // record the start of process one's blocking read, when process two has successfully
      // read process one's write, record the end time (the cpu has switched to the blocked process two)
      clock_gettime(CLOCK_MONOTONIC_RAW, &start1);
      if (read(fildes[CHILD1][READ], &c, sizeof("a") - 1) < 1) { // blocking read
        fprintf(stderr, "Error reading from pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end2);

      // write end time to process two's data pipe (if a start time has been recorded already)
      if (i > 0) {
        if (write(fildes[DATA2][WRITE], &end2, sizeof(end2)) != sizeof(end2)) {
          fprintf(stderr, "Error writing to data pipe in process one. %i: %s.\n", errno, strerror(errno));
          exit(EXIT_FAILURE);
        }
      }

      // write start time of process one's blocking read to process one's data pipe
      if (write(fildes[DATA1][WRITE], &start1, sizeof(start1)) != sizeof(start1)) {
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
    
    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(cpu_set_t), &set);
    fprintf(stdout, "Process two started. PID: %i. CPU: %i\n", getpid(), sched_getcpu());

    char c;
    for (int i = 0; i < MAX_WRITE; i++) {

      if (write(fildes[CHILD1][WRITE], "a", sizeof("a") - 1) != 1) {
        fprintf(stderr, "Error writing to pipe in process two. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      clock_gettime(CLOCK_MONOTONIC_RAW, &start2);
      if (read(fildes[CHILD2][READ], &c, sizeof("b") - 1) < 1) {
        fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end1);

      // write end time to process one's data pipe (if a start time has been recorded already)
      if (i > 0) {
        if (write(fildes[DATA1][WRITE], &end1, sizeof(end2)) != sizeof(end2)) {
          fprintf(stderr, "Error writing to data pipe in process one. %i: %s.\n", errno, strerror(errno));
          exit(EXIT_FAILURE);
        }
      }

      // write start time of process two's blocking read to process one's data pipe
      if (write(fildes[DATA2][WRITE], &start2, sizeof(start1)) != sizeof(start1)) {
        fprintf(stderr, "Error writing to data pipe in process one. %i: %s.\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    fprintf(stdout, "Exiting process two.\n");
    exit(EXIT_SUCCESS);
  }

  waitpid(pid1, 0, 0);
  waitpid(pid2, 0, 0);

  // read the timespecs from the data pipes and print the average
  timespec_t tmp1 = { 0 }, tmp2 = { 0 };
  uint64_t cs_costs[MAX_WRITE * 2] = { 0 };
  uint64_t *tp = cs_costs;

  // read the start and end times and get elapsed nsecs from the DATA1 pipe
  for (int i = 0; i < MAX_WRITE / 2; i++) {
    if (read(fildes[DATA1][READ], &tmp1, sizeof(timespec_t)) != sizeof(timespec_t)) {
      fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (read(fildes[DATA1][READ], &tmp2, sizeof(timespec_t)) != sizeof(timespec_t)) {
      fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    *tp++ = elapsed_nsecs(&tmp1, &tmp2);
  }

  // read the start and end times and get elapsed nsecs from the DATA1 pipe
  for (int i = 0; i < MAX_WRITE / 2; i++) {
    if (read(fildes[DATA2][READ], &tmp1, sizeof(timespec_t)) != sizeof(timespec_t)) {
      fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (read(fildes[DATA2][READ], &tmp2, sizeof(timespec_t)) != sizeof(timespec_t)) {
      fprintf(stderr, "Error reading from pipe in process two. %i: %s.\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    *tp++ = elapsed_nsecs(&tmp1, &tmp2);
  }

  fprintf(stdout, "Average context switch cost: %llu nanoseconds.\n", average_cost(cs_costs));

  close(fildes[CHILD1][READ]);
  close(fildes[CHILD1][WRITE]);
  close(fildes[CHILD2][READ]);
  close(fildes[CHILD2][WRITE]);
  close(fildes[DATA1][READ]);
  close(fildes[DATA1][WRITE]);
  close(fildes[DATA2][READ]);
  close(fildes[DATA2][WRITE]);

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

