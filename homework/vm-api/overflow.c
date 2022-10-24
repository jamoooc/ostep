// Write a program that creates an array of integers called data of size
// 100 using malloc; then, set data[100] to zero. What happens
// when you run this program? What happens when you run this
// program using valgrind? Is the program correct?

// ==20290== Invalid write of size 4
// ==20290==    at 0x109230: main (in /home/jamoooc/code/bin/overflow)
// ==20290==  Address 0x4a0d1d0 is 0 bytes after a block of size 400 alloc'd
// ==20290==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==20290==    by 0x109185: main (in /home/jamoooc/code/bin/overflow)
// ==20290== 
// ==20290== Invalid read of size 4
// ==20290==    at 0x109240: main (in /home/jamoooc/code/bin/overflow)
// ==20290==  Address 0x4a0d1d0 is 0 bytes after a block of size 400 alloc'd
// ==20290==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==20290==    by 0x109185: main (in /home/jamoooc/code/bin/overflow)
// ==20290== 
// p[100]: 42
// ==20290== 
// ==20290== HEAP SUMMARY:
// ==20290==     in use at exit: 400 bytes in 1 blocks
// ==20290==   total heap usage: 2 allocs, 1 frees, 1,424 bytes allocated
// ==20290== 
// ==20290== 400 bytes in 1 blocks are definitely lost in loss record 1 of 1
// ==20290==    at 0x483577F: malloc (vg_replace_malloc.c:299)
// ==20290==    by 0x109185: main (in /home/jamoooc/code/bin/overflow)
// ==20290== 
// ==20290== LEAK SUMMARY:
// ==20290==    definitely lost: 400 bytes in 1 blocks
// ==20290==    indirectly lost: 0 bytes in 0 blocks
// ==20290==      possibly lost: 0 bytes in 0 blocks
// ==20290==    still reachable: 0 bytes in 0 blocks
// ==20290==         suppressed: 0 bytes in 0 blocks
// ==20290== 
// ==20290== For counts of detected and suppressed errors, rerun with: -v
// ==20290== ERROR SUMMARY: 3 errors from 3 contexts (suppressed: 0 from 0)

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int *p = NULL;
  if ((p = malloc(100 * sizeof(int))) == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(1);
  }

  for (int i = 0; i < 100; i++) {
    *(p + i) = i;
  }

  for (int i = 0; i < 100; i++) {
    printf("p[%i]: %i\n", i, p[i]);
  }

  p[100] = 42;
  printf("p[100]: %i\n", p[100]);
}
