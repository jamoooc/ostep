// Write a slight modification of the previous program, this time us-
// ing waitpid() instead of wait(). When would waitpid() be
// useful?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  pid_t pid1 = fork();
  pid_t rv1;
  if (pid1 < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid1 == 0) {
    fprintf(stdout, "Entering child1 PID: %i\n", getpid());

    pid_t pid2 = fork();
    pid_t rv2;
    if (pid2 < 0) {
      fprintf(stdout, "Error forking process.\n");
      exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
      fprintf(stdout, "Entering child2 PID: %i\n", getpid());
      sleep(1);
      fprintf(stdout, "Exiting child2 PID: %i\n", getpid());
    } else {
      fprintf(stdout, "Parent2 waiting for child2\n");
      rv2 = waitpid(pid2, 0, 0);
      fprintf(stdout, "Parent2 PID: %i, rv2: %i\n", getpid(), rv2);
    }

  } else {
    fprintf(stdout, "Parent1 waiting for child1\n");
    rv1 = waitpid(pid1, 0, 0);
    fprintf(stdout, "Parent1 PID: %i, rv1: %i\n", getpid(), rv1);
  }
}
