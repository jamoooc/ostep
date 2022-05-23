// Now write a program that uses wait() to wait for the child process
// to finish in the parent. What does wait() return? What happens if
// you use wait() in the child?

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  pid_t pid = fork();
  pid_t rv;
  if (pid < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "Child PID: %i\n", getpid());
    // rv = wait(NULL); // returns errno: 10: No child processes
    // fprintf(stdout, "wait rv: %i, errno: %i: %s\n", rv, errno, strerror(errno));
  } else {
    rv = wait(NULL); // wait return the child process' PID
    fprintf(stdout, "Parent PID: %i. wait returns %i\n", getpid(), rv);
  }
}



