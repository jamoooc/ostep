#include <stdio.h>
#include <stdlib.h>

// ==20389== Memcheck, a memory error detector
// ==20389== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
// ==20389== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
// ==20389== Command: bin/free_incorrect_ptr
// ==20389== 
// p[5]: 42
// ==20389== Invalid free() / delete / delete[] / realloc()
// ==20389==    at 0x48369AB: free (vg_replace_malloc.c:530)
// ==20389==    by 0x109209: main (in /home/jamoooc/code/bin/free_incorrect_ptr)
// ==20389==  Address 0x4a0d054 is 20 bytes inside a block of size 400 alloc'd
// ==20389==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==20389==    by 0x109195: main (in /home/jamoooc/code/bin/free_incorrect_ptr)
// ==20389== 
// ==20389== 
// ==20389== HEAP SUMMARY:
// ==20389==     in use at exit: 400 bytes in 1 blocks
// ==20389==   total heap usage: 2 allocs, 2 frees, 1,424 bytes allocated
// ==20389== 
// ==20389== LEAK SUMMARY:
// ==20389==    definitely lost: 400 bytes in 1 blocks
// ==20389==    indirectly lost: 0 bytes in 0 blocks
// ==20389==      possibly lost: 0 bytes in 0 blocks
// ==20389==    still reachable: 0 bytes in 0 blocks
// ==20389==         suppressed: 0 bytes in 0 blocks
// ==20389== Rerun with --leak-check=full to see details of leaked memory
// ==20389== 
// ==20389== For counts of detected and suppressed errors, rerun with: -v
// ==20389== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)

int main(int argc, char **argv) {
  int *p = NULL;
  if ((p = malloc(100 * sizeof(int))) == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(1);
  }

  *(p + 5) = 42;

  int *p1 = p + 5;

  printf("p[5]: %i\n", *p1);

  free(p1);
}
