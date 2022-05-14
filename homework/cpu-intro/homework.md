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

3. Switch the order of the processes: -l 1:0,4:100. What happens
now? Does switching the order matter? Why? (As always, use -c
and -p to see if you were right)

Process 1 immediately enters the WAITING state. Process 2 is able to run, and concludes its 4 instructions during the 5 ticks process 1 is performing IO. Process
2 waits one more tick and concludes its IO.

4. We’ll now explore some of the other flags. One important flag is
-S, which determines how the system reacts when a process is-
sues an I/O. With the flag set to SWITCH ON END, the system
will NOT switch to another process while one is doing I/O, in-
stead waiting until the process is completely finished. What hap-
pens when you run the following two processes (-l 1:0,4:100
-c -S SWITCH_ON_END), one doing I/O and the other doing CPU
work?

We would expect process 2 to wait in the READY state for process 1
to complete its IO and to run once process 1 is complete.

5. Now, run the same processes, but with the switching behavior set
to switch to another process whenever one is WAITING for I/O (-l
1:0,4:100 -c -S SWITCH_ON_IO). What happens now? Use -c
and -p to confirm that you are right.

Process 2 is able to run while process 1 is in the WAITING state.

6. One other important behavior is what to do when an I/O com-
pletes. With -I IO_RUN_LATER, when an I/O completes, the pro-
cess that issued it is not necessarily run right away; rather, whatever
was running at the time keeps running. What happens when you
run this combination of processes? (Run ./process-run.py -l
3:0,5:100,5:100,5:100 -S SWITCH ON IO -I IO RUN LATER
-c -p) Are system resources being effectively utilized?

Process 1 runs, and immediately enters the waiting state. Process 2
runs until completion. Process 1 is ready for its next intruction but
waits for process 3 and 4 to complete before being rescheduled. It 
would be more efficient to alternate between process 1 and each of the 
other processes, allowing them to run while process 1 performs its 
IO operations.

7. Now run the same processes, but with -I_IO RUN_IMMEDIATE set,
which immediately runs the process that issued the I/O. How does
this behavior differ? Why might running a process that just com-
pleted an I/O again be a good idea?

We would expect the process with multiple IOs to issue its next IO 
and allow the other processes to run during each of its WAITING 
states. Immediately returning to a process which issued an IO may
be desirable as it's probable the process will either need to issue 
another IO, or to handle the result of the IO.

8. Now run with some randomly generated processes: -s 1 -l 3:50,3:50
or -s 2 -l 3:50,3:50 or -s 3 -l 3:50,3:50. See if you can
predict how the trace will turn out. What happens when you use
the flag -I IO_RUN_IMMEDIATE vs. -I IO_RUN_LATER? What hap-
pens when you use -S SWITCH_ON_IO vs. -S SWITCH_ON_END





