1.
2.
3.

4. For what types of workloads does SJF deliver the same turnaround
times as FIFO?

If a FIFO job queue arrives in order of job length.

5. For what types of workloads and quantum lengths does SJF deliver
the same response times as RR?

The length of jobs would need to match the quantum length of RR for SJF
to deliver the same response time for each job.

6. What happens to response time with SJF as job lengths increase?
Can you use the simulator to demonstrate the trend?

As job length increases the response time increases

./scheduler.py -p SJF -j 3 -s 200 -l 100,200,300 -c
Average -- Response: 133.33  Turnaround 333.33  Wait 133.33

./scheduler.py -p SJF -j 3 -s 200 -l 1000,2000,3000 -c
Average -- Response: 1333.33  Turnaround 3333.33  Wait 1333.33

7. What happens to response time with RR as quantum lengths in-
crease? Can you write an equation that gives the worst-case re-
sponse time, given N jobs?

Response time increase by the length of the scheduler quantum 

`worst case response = (n - 1) * quantum len`
