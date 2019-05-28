#include "../util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT "80"
#define MAXDATASIZE 100

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "usage: talker hostname port\n");
        exit(1);
    }
    int content_length = 1024;
    char content[content_length];
    memset(content, 'a', content_length);
    printf("content: %s\n", content);

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    int rv;
    struct addrinfo *serv_info;
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &serv_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    printf("talker: getaddrinfo:\n");
    print_addrinfo(serv_info);
    struct addrinfo * p;
    int sockfd;
    char s[INET6_ADDRSTRLEN];
    for (p = serv_info; p!= NULL; p = p->ai_next)
    {
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("talker: selecting '%s'-%d\n", s, get_in_port_ntohs(p->ai_addr));
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("talker: selected '%s'-%d, socket: %d\n", s, get_in_port_ntohs(p->ai_addr), sockfd);

    int numbytes;
    // char * content = "hello, this is pipi!";
    if((numbytes = sendto(sockfd, content, strlen(content), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    else
    {
        printf("talker: sendto: %d, '%s'\n", numbytes, content);
    }
    p = NULL;
    freeaddrinfo(serv_info);
    close(sockfd);
    return 0;
}
