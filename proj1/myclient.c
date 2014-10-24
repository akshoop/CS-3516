/*
    Alex Shoop CS 3516 Project 1 http_client
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

#define OPTION "-p"

int main(int argc , char *argv[]) {
    int thisSock, gt, option;
    struct addrinfo thisAddress, *servinfo;
    struct sockaddr_in *h;
    char *message1, *message2, *message3, *parse, *filepath, ip[120], totalmessage[6000], server_reply[65000];
    struct timeval start, end;
    float RTT;
    struct hostent *he;
    in_addr_t data;

    if (argc < 3) {
        fprintf(stderr,"Error: usage is the following: ./http_client [-options] server_url port_number\n");
        exit(1);
    }

    // establish memset
    memset(&thisAddress, 0, sizeof (thisAddress));
    thisAddress.ai_family = AF_INET;
    thisAddress.ai_socktype = SOCK_STREAM;

    //zero out the server address
    bzero((char *)&h, sizeof(h));

    // for case where user does not want RTT time
    if (argc == 3) {
        option = 0;

        parse = strtok(argv[1], "/");

        gt = getaddrinfo(parse, argv[2], &thisAddress, &servinfo);

        if (gt != 0) 
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gt));
            return 1;
        }
    }

    // for case where user wants RTT time
    else if (argc == 4) {
        char *opt = malloc(sizeof(argv[1]));
        strcpy(opt, argv[1]);

        // make sure the correct option is used
        if (strcmp(opt, OPTION)) { 
            fprintf(stderr, "Error: option is not -p\n");
            exit(1);
        }

        parse = strtok(argv[2], "/");

        option = 1;

        gt = getaddrinfo(parse, argv[3], &thisAddress, &servinfo);

        if (gt != 0) 
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gt));
            exit(1);
        }
    }

    // setup socket
    if ((thisSock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0) {
        close(thisSock);
        fprintf(stderr, "Error: socket error\n");
        exit(1);
            //continue;
    }

    gettimeofday(&start, NULL);

    // connect to  server
    if (connect(thisSock , servinfo->ai_addr, servinfo->ai_addrlen) < 0) {  
        close(thisSock);
        fprintf(stderr,"Error: connect error\n");
        exit(1);
    }

    gettimeofday(&end, NULL);

    puts("Connected\n");

    freeaddrinfo(servinfo);

    // send data
    filepath = strtok(NULL, "/");
    if (filepath == NULL) {
        filepath = "index.html";
    }
    message1 = "GET /";
    message2 = " HTTP/1.1\r\nHost: ";
    message3 = "\r\nConnection: close\r\n\r\n";
    sprintf(totalmessage, "%s%s%s%s%s", message1, filepath, message2, parse, message3);
    
    if (send(thisSock, totalmessage, strlen(totalmessage), 0) < 0) {
        fprintf(stderr, "Error: send error\n");
        exit(1);
    }

    puts("Data Send\n");
     
    // receive data
    int i = 1;
    while (i) {
        i = recv(thisSock, server_reply, sizeof (server_reply), MSG_DONTWAIT);
        printf("%s", server_reply);
    }

    if (option == 1) {
        RTT = (((end.tv_sec - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)/1000));
        printf("RTT in milliseconds: %f\n", RTT);
    }

    close(thisSock);
    return 0;
} 
