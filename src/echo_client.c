/******************************************************************************
 * echo_client.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo client.  The  *
 *              client connects to an arbitrary <host,port> and sends input    *
 *              from stdin.                                                    *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096
// #define O_RDONLY 00

int main(int argc, char *argv[])
{
    //    if (argc != 3)
    //    {
    //        fprintf(stderr, "usage: %s <server-ip> <port>",argv[0]);
    //        return EXIT_FAILURE;
    //    }
    // if(argc!=4){
    //     fprintf(stderr,"usage: %s <server-ip> <port> <file-path>\n",argv[0]);
    //     return EXIT_FAILURE;
    //     char buf[BUF_SIZE];
    //	}
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <server-ip> <port> <file-path>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char buf[BUF_SIZE];
    int status, sock;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    struct addrinfo *servinfo;       // will point to the results
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s \n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        fprintf(stderr, "Socket failed");
        return EXIT_FAILURE;
    }

    if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        fprintf(stderr, "Connect");
        return EXIT_FAILURE;
    }

    //     FILE *fp=fopen(argv[3],"r");
    //     char msg[BUF_SIZE];
    //     char temp[BUF_SIZE];

    //     int pos=0;
    //     for(;fgets(temp,sizeof(temp),fp)!=NULL;){
    //         int len=strlen(temp);
    //         memcpy(msg+pos,temp,len);
    //         pos+=len;
    //     }
    //     msg[pos]='\0';
    //     fclose(fp);

    //     int bytes_received;
    //     fprintf(stdout, "Sending %s", msg);
    //     send(sock, msg , strlen(msg), 0);
    //     if((bytes_received = recv(sock, buf, BUF_SIZE, 0)) > 1)
    //     {
    //         buf[bytes_received] = '\0';
    //         fprintf(stdout, "Received %s", buf);
    //     }

    //     freeaddrinfo(servinfo);
    //     close(sock);
    //     return EXIT_SUCCESS;
    // }
    char msg0[BUF_SIZE];
    memset(msg0, 0, sizeof(msg0));
    int fd_in = open(argv[3], 00);
    if (fd_in < 0)
    {
        printf("Failed to open the file\n");
        return EXIT_FAILURE;
    }
    int readRet = read(fd_in, msg0, 8192);
    close(fd_in);
    int bytes_received1;
    fprintf(stdout, "Sending %s", msg0);
    send(sock, msg0, strlen(msg0), 0);
    if ((bytes_received1 = recv(sock, buf, BUF_SIZE, 0)) > 1)
    {
        buf[bytes_received1] = '\0';
        fprintf(stdout, "Received %s", buf);
    }
    freeaddrinfo(servinfo);
    close(sock);
    return EXIT_SUCCESS;
}
