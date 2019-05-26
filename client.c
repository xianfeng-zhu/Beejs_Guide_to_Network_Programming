#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT "3490"
#define MAXDATASIZE 100

int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rv;
    struct addrinfo *serv_info;
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &serv_info)) != 0)
    {
        printf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    printf("client: getaddrinfo:\n");
    print_addrinfo(serv_info);
    struct addrinfo * p;
    int sockfd;
    char s[INET6_ADDRSTRLEN];
    for (p = serv_info; p!= NULL; p = p->ai_next)
    {
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("client: selecting '%s'-%d\n", s, get_in_port_ntohs(p->ai_addr));
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: selected '%s'-%d\n", s, get_in_port_ntohs(p->ai_addr));
    
    p = NULL;
    freeaddrinfo(serv_info);

    int numbytes;
    char buf[MAXDATASIZE];
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("client: recv '%s': %d\n", buf, numbytes);
    close(sockfd);
    return 0;
}
