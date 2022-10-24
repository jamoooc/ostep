// Try out some of the other interfaces to memory allocation. For ex-
// ample, create a simple vector-like data structure and related rou-
// tines that use realloc() to manage the vector. Use an array to
// store the vectors elements; when a user adds an entry to the vec-
// tor, use realloc() to allocate more space for it. How well does
// such a vector perform? How does it compare to a linked list? Use
// valgrind to help you find bugs

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

  // allocate an array of ptr to char
  size_t vlen = 3;
  char **vec = NULL;
  if ((vec = malloc(sizeof(char *) * vlen)) == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(1);
  }

  size_t len = 0;
  size_t line_size = 0;
  size_t nitems = 0;
  char **vref = vec;

  while (nitems < 10) {
    
    char *line = NULL;
    printf("Enter item: ");
    line_size = getline(&line, &len, stdin);

    if (nitems == vlen) {
      printf("Vector capacity reached. Re-allocating.\n");
      vlen++;
      if ((vec = realloc(vref, sizeof(char *) * vlen)) == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        exit(1);
      }
      vref = vec;
    }
    *(vec + nitems) = line;
    nitems++;
  }

  for (int i = 0; i < vlen; i++) {
    printf("Item: %s", *(vref + i));
    free(*(vref + i));
  }
  free(vec);
}

