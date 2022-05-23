// Write a program that creates two children, and connects the stan-
// dard output of one to the standard input of the other, using the
// pipe() system call.

// child1 stdin is the read end of the pipe
// child2 stdou is the write end of the pipe
// expect child 1 to be able to read message from stdin sent by child2 to stdout

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

enum {
  READ,
  WRITE
};

int main(void) {

  int fildes[2];
  pipe(fildes);

  // set read fd non blocking
  int flags = fcntl(fildes[0], F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fildes[0], F_SETFL, flags);

  pid_t pid1 = fork();
  if (pid1 < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid1 == 0) {
    fprintf(stdout, "Child1 PID: %i.\n", getpid());

    // close stdin and assign descriptor to READ pipe
    dup2(fildes[READ], STDIN_FILENO);

    pid_t pid2 = fork();
    if (pid2 < 0) {
      fprintf(stdout, "Error forking process.\n");
      exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
      fprintf(stdout, "Child2 PID: %i.\n", getpid());
      
      dup2(fildes[WRITE], STDOUT_FILENO);
      fprintf(stdout, "hello from child2");

    } else {
      waitpid(pid2, 0, 0);

      // read message sent by child 2
      char c;
      while ((read(STDIN_FILENO, &c, 1)) > 0) {
        fprintf(stdout, "%c", c);
      }
      puts("");
    }

  } else {
    waitpid(pid1, 0, 0);
    fprintf(stdout, "Child1 and child2 complete.\n");
  }
}




