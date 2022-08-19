// Next, create a little program that uses a certain amount of memory,
// called memory-user.c. This program should take one command-
// line argument: the number of megabytes of memory it will use.
// When run, it should allocate an array, and constantly stream through
// the array, touching each entry. The program should do this indefi-
// nitely, or, perhaps, for a certain amount of time also specified at the
// command line.

// 4. Now, while running your memory-user program, also (in a dif-
// ferent terminal window, but on the same machine) run the free
// tool. How do the memory usage totals change when your program
// is running? How about when you kill the memory-user program?
// Do the numbers match your expectations? Try this for different
// amounts of memory usage. What happens when you use really
// large amounts of memory?

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  fprintf(stdout, "memory-user. PID: %i\n", getpid());

  size_t mebi = 1024;

  char *arr = NULL;
  if ((arr = malloc(sizeof(char) * (mebi * 1024 * 1024))) == NULL) {
    exit(EXIT_FAILURE);
  }
  fprintf(stdout, "Allocated array\n");

  size_t i = 0;
  char *p = arr;
  char a = 'a';

  while (i < (mebi * 1024 * 1024)) {
    *(p + i++) = a;
    if (i == (mebi * 1024 * 1024)) {
      i = 0;
    }
    fprintf(stdout, "%zu %c\n", i, *p);
    sleep(1);
  }
}
