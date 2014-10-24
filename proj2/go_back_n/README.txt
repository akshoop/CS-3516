README for Alex Shoop CS3516 Project 2 - Go-Back-N protocol

To compile:
make

To clean:
make clean

To run:
./p2_gbn

Check gbn_example1_trace and altbit_example2_trace PDF files to see example
runs of the Go-Back-N protocol simulation program.

Example1 is where 25 messages are simulated, with no loss, corruption, or out-of-order
probability.
The average time between messages from sender's layer 5 is 10.
Also no randomization nor bidirectional option.
From the results, all 25 messages get received, even with 10 average time.

Example2 is where 25 messages are simulated, with 0.2 loss probability,
0.2 corruption probability, and 0 out-of-order probability.
The average time between messages from sender's layer 5 is 10.
Randomization is set, while bidirectional is not set.
As seen in results, only all 25 messages also get received.
