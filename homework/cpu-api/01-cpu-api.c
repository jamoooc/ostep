// Write a program that calls fork(). Before calling fork(), have the
// main process access a variable (e.g., x) and set its value to some-
// thing (e.g., 100). What value is the variable in the child process?
// What happens to the variable when both the child and parent change
// the value of x?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

  volatile int count = 100;
  fprintf(stdout, "Parent process. pid: %i count: %i\n", getpid(), count);

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "In child process. pid: %i count: %i\n", getpid(), count);
    count -= 10;
    fprintf(stdout, "Exiting child process. pid: %i count: %i\n", getpid(), count);
  } else {
    fprintf(stdout, "In parent process. pid: %i count: %i\n", getpid(), count);
    waitpid(pid, 0, 0); // guarantee parent modifies count after child
    count += 10;
    fprintf(stdout, "Exiting parent process. pid: %i count: %i\n", getpid(), count);
  }

  fprintf(stdout, "Parent process. pid: %i count: %i\n", getpid(), count);
  exit(EXIT_SUCCESS);
}
