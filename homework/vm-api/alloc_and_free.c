// Create a program that allocates an array of integers (as above), frees
// them, and then tries to print the value of one of the elements of
// the array. Does the program run? What happens when you use
// valgrind on it?

// ==20318== Memcheck, a memory error detector
// ==20318== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
// ==20318== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
// ==20318== Command: bin/alloc_and_free
// ==20318== 
// ==20318== Invalid read of size 4
// ==20318==    at 0x1091DB: main (in /home/jamoooc/code/bin/alloc_and_free)
// ==20318==  Address 0x4a0d040 is 0 bytes inside a block of size 400 free'd
// ==20318==    at 0x48369AB: free (vg_replace_malloc.c:530)
// ==20318==    by 0x1091D6: main (in /home/jamoooc/code/bin/alloc_and_free)
// ==20318==  Block was alloc'd at
// ==20318==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==20318==    by 0x109195: main (in /home/jamoooc/code/bin/alloc_and_free)
// ==20318== 
// p[0]: 0
// ==20318== 
// ==20318== HEAP SUMMARY:
// ==20318==     in use at exit: 0 bytes in 0 blocks
// ==20318==   total heap usage: 2 allocs, 2 frees, 1,424 bytes allocated
// ==20318== 
// ==20318== All heap blocks were freed -- no leaks are possible
// ==20318== 
// ==20318== For counts of detected and suppressed errors, rerun with: -v
// ==20318== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int *p = NULL;
  if ((p = malloc(100 * sizeof(int))) == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(1);
  }

  free(p);

  printf("p[0]: %i\n", p[0]);
}
