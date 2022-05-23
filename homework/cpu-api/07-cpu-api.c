// Write a slight modification of the previous program, this time us-
// ing waitpid() instead of wait(). When would waitpid() be
// useful?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  pid_t pid1 = fork();
  if (pid1 < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid1 == 0) {
    
    fprintf(stdout, "Closing STDOUT_FILENO.\n");
    close(STDOUT_FILENO);
    fprintf(stdout, "This won't print.\n");
    printf("Same with standard printf.\n");

  } else {
    waitpid(pid1, 0, 0);
    fprintf(stdout, "This will print. STDOUT_FILENO only closed in child.\n");
  }
}
