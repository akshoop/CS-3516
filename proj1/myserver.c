/*
    Alex Shoop CS 3516 Project 1 http_server
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

 
int main(int argc , char *argv[]) {
    int thisSock, newSock, temp;
    struct addrinfo thisServer, *servinfo;
    struct sockaddr_in theirClient;
    char *message1, *message2, totalmessage[6000], server_reply[6000];
    struct timeval start, end;
    long RTT;

    if (argc < 0) {
        fprintf(stderr,"Error: usage is the following: ./http_server port_number\n");
        exit(1);
    }

     // establish memset
    memset(&thisServer, 0, sizeof (thisServer));
    thisServer.ai_family = AF_INET;
    thisServer.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, argv[1], &thisServer, &servinfo);

    // setup socket
    if ((thisSock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
        close(thisSock);
        fprintf(stderr, "Error: socket error\n");
        exit(1);
    }

    //zero out the server address
    bzero((char *)&thisServer, sizeof(thisServer));

    if (bind(thisSock, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        fprintf(stderr, "Error: bind error\n");
        exit(1);
    }
    printf("Bind success\n");

    while (1) {

        printf("Waiting for client...\n");

        if (listen(thisSock, 1) < 0) {
            fprintf(stderr, "Error: listen error\n");
            exit(1);
        }
        temp = sizeof(struct sockaddr_in);
        newSock = accept(thisSock, (struct sockaddr *)&theirClient, (socklen_t*)&temp);
        if (newSock < 0) {
            fprintf(stderr, "Error: accept error\n");
            exit(1);
        }

        char tempbuff[65000];
        memset(tempbuff, 0, sizeof (tempbuff));

        int i = recv(newSock, tempbuff, sizeof (tempbuff), 0);
        if (i < 0) {
            fprintf(stderr, "Error: receive error\n");
            exit(1);
        }

        message1 = "HTTP/1.1 200 OK\r\n\r\n";

        send(newSock, message1, sizeof(message1), 0);

        FILE *fp;

        fopen("TMDG.html", "r");
        while(fgets(server_reply, 6000, fp) != NULL);
        fclose(fp);

        puts(server_reply);

    }
    freeaddrinfo(servinfo);
    close(thisSock);
    close(newSock);
    printf("Finished\n");
    return 0;
}
