// 10 INTERLUDE: MEMORY API

// 1. First, write a simple program called null.c that creates a pointer
// to an integer, sets it to NULL, and then tries to dereference it. Com-
// pile this into an executable called null. What happens when you
// run this program?

// 2. Next, compile this program with symbol information included (with
// the -g flag). Doing so let’s put more information into the exe-
// cutable, enabling the debugger to access more useful information
// about variable names and the like. Run the program under the de-
// bugger by typing gdb null and then, once gdb is running, typing
// run. What does gdb show you?

// 3. Finally, use the valgrind tool on this program. We’ll use the memcheck
// tool that is a part of valgrind to analyze what happens. Run
// this by typing in the following: valgrind --leak-check=yes
// null. What happens when you run this? Can you interpret the
// output from the tool?

// gcc -g -o null null.c

// Starting program: /home/jamoooc/code/bin/null 
// Program received signal SIGSEGV, Segmentation fault.
// 0x0000555555555150 in main (argc=1, argv=0x7fffffffe4b8) at null.c:5
// 5         int i = *ptr_to_null;

// ==19697== Memcheck, a memory error detector
// ==19697== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
// ==19697== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
// ==19697== Command: bin/null
// ==19697== 
// ==19697== Invalid read of size 4
// ==19697==    at 0x109150: main (null.c:5)
// ==19697==  Address 0x0 is not stack'd, malloc'd or (recently) free'd
// ==19697== 
// ==19697== 
// ==19697== Process terminating with default action of signal 11 (SIGSEGV)
// ==19697==  Access not within mapped region at address 0x0
// ==19697==    at 0x109150: main (null.c:5)
// ==19697==  If you believe this happened as a result of a stack
// ==19697==  overflow in your program's main thread (unlikely but
// ==19697==  possible), you can try to increase the size of the
// ==19697==  main thread stack using the --main-stacksize= flag.
// ==19697==  The main thread stack size used in this run was 8388608.
// ==19697== 
// ==19697== HEAP SUMMARY:
// ==19697==     in use at exit: 0 bytes in 0 blocks
// ==19697==   total heap usage: 0 allocs, 0 frees, 0 bytes allocated
// ==19697== 
// ==19697== All heap blocks were freed -- no leaks are possible
// ==19697== 
// ==19697== For counts of detected and suppressed errors, rerun with: -v
// ==19697== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)

#include <stdio.h>

int main(int argc, char **argv) {
  int *ptr_to_null = NULL;
  int i = *ptr_to_null;
  printf("i: %i\n", i);
}
