# simulation

## questions

1. Run process-run.py with the following flags: -l 5:100,5:100.
What should the CPU utilization be (e.g., the percent of time the
CPU is in use?) Why do you know this? Use the -c and -p flags to
see if you were right.

100%. Neither process performs IO so each runs to completion before switching.

2. Now run with these flags: ./process-run.py -l 4:100,1:0.
These flags specify one process with 4 instructions (all to use the
CPU), and one that simply issues an I/O and waits for it to be done.
How long does it take to complete both processes? Use -c and -p
to find out if you were right

IO_LENGTH defaults to 5 ticks. We should expect 4 instructions for process one
and 7 ticks for process 2 (1 for RUN:io, one for RUN:io_done and and 5 ticks WAITING).
If we scheduled process 2 first `./process-run.py -l 1:0,4:100 -p -c`, it could run 
during process 1's WAITING state.




