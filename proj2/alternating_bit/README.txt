README for Alex Shoop CS3516 Project 2 - alternating bit protocol

To compile:
make

To clean:
make clean

To run:
./p2_altbit

Check altbit_trace_example1 and altbit_trace_example2 PDF files to see example
runs of the alternating bit protocol simulation program.

Example1 is where 100 messages are simulated, with no loss, corruption, or out-of-order
probability.
The average time between messages from sender's layer 5 is 1000.
I used 1000 because lesser values such as 10 did not allow all messages to be received.
Also no randomization nor bidirectional option.
From the results, all 100 messages get received.

Example2 is where 100 messages are simulated, with 0.2 loss probability,
0.2 corruption probability, and 0 out-of-order probability.
The average time between messages from sender's layer 5 is 100.
Randomization is set, while bidirectional is not set.
As seen in results, only 76 messages get received.
