// Write a program that opens a file (with the open() system call)
// and then calls fork() to create a new process. Can both the child
// and parent access the file descriptor returned by open()? What
// happens when they are writing to the file concurrently, i.e., at the
// same time?

// The parent and child process seem to alternate quite consistently 
// between writing 4096 bytes, occasionally another power of 2, before there
// is a context switch (except in their last iteration which writes the
// remaining bytes).

// gcc -o fork2 fork2.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

  FILE *fp = NULL;
  if ((fp = fopen("./file.txt", "w+")) == NULL) {
    fprintf(stdout, "Error creating file.\n");
    exit(EXIT_FAILURE);
  };
  fprintf(stdout, "Parent process. PID: %i, fp: %p, fp: %p\n", getpid(), fp, &fp);

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stdout, "Error forking process.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "\nIn child process. pid: %i\n", getpid());

    for (int i = 0; i < 100000; i++) {
      fwrite("1", 1, 1, fp);
    }

    fprintf(stdout, "\nChild complete. pid: %i\n", getpid());
  } else {
    fprintf(stdout, "\nIn parent process. pid: %i\n", getpid());

    for (int i = 0; i < 100000; i++) {
      fwrite("2", 1, 1, fp);
    }

    fprintf(stdout, "Parent complete. pid: %i\n", getpid());

    // wait for the child process if not complete
    waitpid(pid, 0, 0);
  }

  char c = '\0';
  char cur = '\0';
  int count = 0;
  fseek(fp, 0, SEEK_SET);
  while (fread(&c, 1, 1, fp) != 0) {
    if (cur != c) {
      if (cur != '\0') {
        fprintf(stdout, "%s wrote %i items\n", cur == '1' ? "child" : "parent", count);
      }
      cur = c;
      count = 0;
    }
    count++;
    // fprintf(stdout, "%c", c);
  }
  fprintf(stdout, "%s wrote %i items\n", cur == '1' ? "child" : "parent", count);
  puts("");

  exit(EXIT_SUCCESS);
}
