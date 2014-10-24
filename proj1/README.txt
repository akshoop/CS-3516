Alexander Shoop (akshoop) CS 3516 README file

How to make:
make

It should execute the makefile which creates http_client and http_server.
The makefile runs the following:
gcc -c myclient.c
gcc myclient.o -o http_client
gcc -c myserver.c
gcc myserver.o -o http_server

Running the http_client program:

The usage for http_client is ./http_client [-options] server_address port_number.
For options, only -p is a valid option which gives the RTT of the requested server.

An example run of the http_client program is: ./http_client www.mit.edu 80.

It should print the entire index.html file of www.mit.edu.

Unfortunately my http_client program does not function if a user just
types in an IP address such as MIT's 23.205.14.184.

There is a PDF of 10 different website accessed via my http_client program.



Running the http_server program:

The usage for http_server is ./http_server port_number.

Unfortunately I was unable to fully accomplish the purpose of the server program.
Currently it stops at waiting for client, which means the listen() function
is running.
But even running from a web browser with the same port number, my http_server
program does not respond.

Please look at the source code. I would appreciate the grading considerations.
