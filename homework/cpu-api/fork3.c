// Write another program using fork(). The child process should
// print “hello”; the parent process should print “goodbye”. You should
// try to ensure that the child process always prints first; can you do
// this without calling wait() in the parent?


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "Child: hello\n");
  } else {
    // int t = 0;
    // for (int i = 0; i < 100000; i++) {
    //   i % 2 == 0 ? t++ : t--;
    // }
    sleep(1);
    fprintf(stdout, "Parent: goodbye\n");
  }
  exit(EXIT_SUCCESS);
}





