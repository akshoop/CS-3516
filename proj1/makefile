# Alex Shoop makefile for CS 3516 Project 1

all: http_client http_server

http_server: myserver.o
	gcc myserver.o -o http_server

http_client: myclient.o
	gcc myclient.o -o http_client

myserver.o: myserver.c
	gcc -c myserver.c

myclient.o: myclient.c
	gcc -c myclient.c

clean:
	rm -f *.o http_client http_server
