/******************************************************************************
 * echo_server.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo server.  The  *
 *              server runs on a hard-coded port and simply write back anything*
 *              sent to it by connected clients.  It does not support          *
 *              concurrent clients.                                            *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "parse.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096
#define GET_SUCCESS 1
#define GET_FAILURE 0
#define URL_MAX_SIZE 256

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

char RESPONSE_400[] = "HTTP/1.1 400 Bad request\r\n\r\n";
char RESPONSE_404[] = "HTTP/1.1 404 Not Found\r\n\r\n";
char RESPONSE_501[] = "HTTP/1.1 501 Not Implemented\r\n\r\n";
char RESPONSE_505[] = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
char RESPONSE_200[] = "HTTP/1.1 200 OK\r\n\r\n";

char http_version_now[] = "HTTP/1.1"; 
char root_path[] = "./static_site";
char file_path[] = "/index.html";

int http_get(Request* request, char* URL, int client_sock)
{
    struct stat file_state;
    if(stat(URL, &file_state) == -1)
        return GET_FAILURE;
    
    int fd_in = open(URL, O_RDONLY);
    if(!S_ISREG(file_state.st_mode) || !(file_state.st_mode & S_IRUSR))
        return GET_FAILURE;

    if(fd_in < 0)
    {
        printf("Failed to open the file\n");
        return GET_FAILURE;
    }
    
    char response1[BUF_SIZE];
    char response2[BUF_SIZE];
    read(fd_in, response2, BUF_SIZE);
    strcpy(response1, RESPONSE_200);
    strcat(response1, response2);
    send(client_sock, response1, strlen(response1), 0);
   
    close(fd_in);
    return GET_SUCCESS;
}

int http_head(Request* request, char* URL, int client_sock)
{
    struct stat file_state;
    if(stat(URL, &file_state) == -1)
    {
        return GET_FAILURE;
    }
    if(!S_ISREG(file_state.st_mode) || !(file_state.st_mode & S_IRUSR))
    {    
        return GET_FAILURE;
    }
    int fd_in = open(URL, O_RDONLY);
    if(fd_in < 0)
    {
        printf("Failed to open the file\n");
        return GET_FAILURE;
    }

    char response[BUF_SIZE];
    strcpy(response, RESPONSE_200);

    if (send(client_sock, response, strlen(response), 0) != strlen(response))
    {
        close_socket(client_sock);
        fprintf(stderr, "Error sending to client.\n");
        return GET_FAILURE;
    }
    close(fd_in);
    return GET_SUCCESS;
}

int main(int argc, char *argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    fprintf(stdout, "----- Echo Server -----\n");

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }

        readret = 0;
        memset(buf, 0, BUF_SIZE);

        while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
        {
            Request *request = parse(buf, readret, client_sock);
            if (request == NULL)
            {
                send(client_sock, RESPONSE_400, strlen(RESPONSE_400), 0);
            }
            else if (strcmp(request->http_version, http_version_now) != 0)
            {
                send(client_sock, RESPONSE_505, strlen(RESPONSE_505), 0);
            }
            else if (strcmp(request->http_method, "POST") == 0)
            {
                send(client_sock, RESPONSE_200, strlen(RESPONSE_200), 0);
            }
            else if (strcmp(request->http_method, "GET") == 0)
            {
                char get_URL[URL_MAX_SIZE];
                memset(get_URL, 0, sizeof(get_URL));
                strcat(get_URL, root_path);
                if (strcmp(request->http_uri, "/") == 0)
                    strcat(get_URL, file_path);
                else if (strlen(request->http_uri) + strlen(root_path) < URL_MAX_SIZE)
                    strcat(get_URL, request->http_uri);
                else
                {
                    send(client_sock, RESPONSE_404, strlen(RESPONSE_404), 0);
                    continue;
                }

                if (http_get(request, get_URL, client_sock) == GET_FAILURE)
                {
                    send(client_sock, RESPONSE_404, strlen(RESPONSE_404), 0);
                }
            }
            else if (strcmp(request->http_method, "HEAD") == 0)
            {
                char head_URL[URL_MAX_SIZE];
                memset(head_URL, 0, sizeof(head_URL));
                strcat(head_URL, root_path);
                if (strcmp(request->http_uri, "/") == 0)
                    strcat(head_URL, file_path);
                else if (strlen(request->http_uri) + strlen(root_path) < URL_MAX_SIZE)
                    strcat(head_URL, request->http_uri);
                else
                {
                    send(client_sock, RESPONSE_404, strlen(RESPONSE_404), 0);
                    continue;
                }

                if (http_head(request, head_URL, client_sock) == GET_FAILURE)
                {
                    send(client_sock, RESPONSE_404, strlen(RESPONSE_404), 0);
                }
            }
            else
            {
                send(client_sock, RESPONSE_501, strlen(RESPONSE_501), 0);
            }

            free(request->headers);
            free(request);
            memset(buf, 0, BUF_SIZE);
        }

        if (readret == -1)
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}