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
#define MAXBUFLEN (1024*1024)

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
        fprintf(stderr, "usage listener port");
        exit(1);
    }
    
    int sockfd;
    struct addrinfo hints, * serv_info;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &serv_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    printf("listener: getaddrinfo:\n");
    print_addrinfo(serv_info);
    struct addrinfo * p = serv_info;
    char s[INET6_ADDRSTRLEN];
    for (; p != NULL; p = p->ai_next)
    {
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("listener: trying create socket with: '%s'-%d\n", s, get_in_port_ntohs(p->ai_addr));
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("listener: socket");
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
            perror("listener: bind");
            continue;
        }
        printf("listener: done with socket: %d!\n", sockfd);
        break;
    }

    if(p == NULL)
    {
        fprintf(stderr, "listener: failed to bind\n");
        exit(1);
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)(p->ai_addr)), s, sizeof s);
    printf("listener: bind on socket: %d, '%s'-%d\n", 
        sockfd, 
        s, 
        get_in_port_ntohs(p->ai_addr));
    p = NULL;
    freeaddrinfo(serv_info);
    
    struct sockaddr_storage their_addr;
    struct sockaddr * p_their_addr = (struct sockaddr *)(&their_addr);
    int addrlen = sizeof their_addr;
    int numbytes;
    char buf[MAXBUFLEN];
    while (1)
    {
        memset(p_their_addr, 0, addrlen);
        numbytes = 0;
        printf("\nlistener: waiting to recvfrom...\n");
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)(&their_addr), &addrlen)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }
        inet_ntop(p_their_addr->sa_family, get_in_addr(p_their_addr), s, sizeof s);
        if (numbytes >= 0)
        {
            buf[numbytes] = '\0';
        }
        else
        {
            buf[0] = '\0';
        }
        // printf("listener: '%s'\n", buf);
        printf("listener: got packet from '%s'-%d, len: %d, '%s'\n", s, get_in_port_ntohs(p_their_addr), numbytes, buf);
    }
    close(sockfd);
    return 0;
}
