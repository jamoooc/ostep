// Write a simple program that allocates memory using malloc() but
// forgets to free it before exiting. What happens when this program
// runs? Can you use gdb to find any problems with it? How about
// valgrind (again with the --leak-check=yes flag)?

// ==19996== Memcheck, a memory error detector
// ==19996== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
// ==19996== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
// ==19996== Command: bin/no_free
// ==19996== 
// ==19996== 
// ==19996== HEAP SUMMARY:
// ==19996==     in use at exit: 4 bytes in 1 blocks
// ==19996==   total heap usage: 1 allocs, 0 frees, 4 bytes allocated
// ==19996== 
// ==19996== 4 bytes in 1 blocks are definitely lost in loss record 1 of 1
// ==19996==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==19996==    by 0x10914D: main (in /home/jamoooc/code/bin/no_free)
// ==19996== 
// ==19996== LEAK SUMMARY:
// ==19996==    definitely lost: 4 bytes in 1 blocks
// ==19996==    indirectly lost: 0 bytes in 0 blocks
// ==19996==      possibly lost: 0 bytes in 0 blocks
// ==19996==    still reachable: 0 bytes in 0 blocks
// ==19996==         suppressed: 0 bytes in 0 blocks
// ==19996== 
// ==19996== For counts of detected and suppressed errors, rerun with: -v
// ==19996== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int *p = malloc(sizeof(int));
  *p = 1;
}
