#include "../base/base.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

// #define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s)
{
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        errno = saved_errno;
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage server port");
        exit(1);
    }
    
    int sockfd;
    struct addrinfo hints, * serv_info;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &serv_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    printf("server: getaddrinfo:\n");
    print_addrinfo(serv_info);
    struct addrinfo * p = serv_info;
    char s[INET6_ADDRSTRLEN];
    for (; p != NULL; p = p->ai_next)
    {
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("server: trying '%s'-%d\n", s, get_in_port_ntohs(p->ai_addr));
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        // int yes = 1;
        // if (setsocketopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) == -1))
        // {
        //     perror("setsocketopt");
        //     exit(1);
        // }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        printf("server: done!\n");
        break;
    }

    if(p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)(p->ai_addr)), s, sizeof s);
    printf("server: bind on socket: %d, '%s'-%d\n", 
        sockfd, 
        s, 
        get_in_port_ntohs(p->ai_addr));
    if(listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("server: listening on socket: %d, '%s'-%d\n", 
        sockfd, 
        s, 
        get_in_port_ntohs(p->ai_addr));
    p = NULL;
    freeaddrinfo(serv_info);

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for incoming connections...\n\n");
    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    int new_fd;
    while (1)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)(&their_addr), &sin_size);
        if(new_fd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from '%s'-%d\n", s, get_in_port_ntohs((struct sockaddr *)(&their_addr)));
        if(!fork())
        {
            printf("server: run in child process: %d, client socket: %d\n", getpid(), new_fd);
            close(sockfd);
            char * content = "hello, this is xf!";
            int numbytes;
            if((numbytes = send(new_fd, content, strlen(content), 0)) == -1)
            {
                perror("send");
            }
            else
            {
                printf("server: send: '%s': %d\n", content, numbytes);
            }
            char buf[MAXDATASIZE];
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
            {
                perror("server: recv");
            }
            if(numbytes > 0)
            {
                buf[numbytes] = '\0';
                printf("server: recv: '%s': %d\n", buf, numbytes);
            }
            close(new_fd);
            printf("server: client socket %d closed\n\n", new_fd);
            exit(0);
        }
        close(new_fd);
    }
    return 0;
}
