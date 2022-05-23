// Write a program that calls fork() and then calls some form of
// exec() to run the program /bin/ls. See if you can try all of the
// variants of exec(), including (on Linux) execl(), execle(),
// execlp(), execv(), execvp(), and execvpe(). Why do
// you think there are so many variants of the same basic call?

// execl, execle, execlp, execv, execvp, execvP - all variants of execve

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void call_execvp();
void call_execl();
void call_execle();
void call_execlp();
void call_execvP();

extern char **environ;

int main(void) {

  // print the processes environment
  // char *str = NULL;
  // while ((str = *environ++) != NULL) {
  //   fprintf(stdout, "%s\n", str);
  // }
  // fprintf(stdout, "\n END ENVIRON \n");

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "Child\n");
    
    // call_execvp();
    // call_execl();
    // call_execle();
    // call_execlp();
    call_execvP();

  }
  exit(EXIT_SUCCESS);
}

void call_execvp() {
  // searches PATH for executable file
  char *argv[] = { "ls", NULL };
  if ((execvp(argv[0], argv)) < 0) {
    fprintf(stderr, "Error creating new process. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void call_execl() {
  // does not search PATH, requires filename e.g. /bin/ls not ls
  char *argv[] = { "/bin/ls", NULL };
  if ((execl(argv[0], argv[0], argv[1])) < 0) {
    fprintf(stderr, "Error creating new process. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void call_execle() {
  // execle allows us to specify the environment (the process' environment variables)
  // the other exec functions use the extern char **environ variable
  // https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
  char *env[] = { "TEST=test", NULL };
  if ((execle("/usr/bin/printenv", "printenv", NULL, env)) < 0) {
    fprintf(stderr, "Error creating new process. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void call_execlp() {
  // execlp will search PATH for an executable file unless it contains a '/'
  if ((execlp("ls", "ls", NULL)) < 0) {
    fprintf(stderr, "Error creating new process. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void call_execvP() {
  // execvP will search 'search_path' for an executable file
  char *argv[] = { "ls", NULL };
  char *search_path = "/usr/bin:/bin";
  if ((execvP("ls", search_path, argv)) < 0) {
    fprintf(stderr, "Error creating new process. %i: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}


