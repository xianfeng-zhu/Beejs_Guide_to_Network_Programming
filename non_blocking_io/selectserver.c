#include "../base/base.h"
#include <unistd.h>
#include <stdlib.h>

#define PORT "9034"

int main(int argc, char const *argv[])
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rv;
    struct addrinfo *ai;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    printf("got local addr info:\n");
    print_addrinfo(ai);

    struct addrinfo *p;
    int server_sock;
    for (p = ai; p != NULL; p = p->ai_next)
    {
        server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_sock < 0)
        {
            continue;
        }
        int yes = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(server_sock, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(server_sock);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
    print_sockaddr("selected: ", p->ai_addr);
    freeaddrinfo(ai);

    if (listen(server_sock, 10) == -1)
    {
        perror("listen");
        exit(3);
    }

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(server_sock, &master);
    int fd_max = server_sock;

    struct timeval timeout;
    for (;;)
    {
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        read_fds = master;
        printf("waiting on select...\n");
        int sel = select(fd_max + 1, &read_fds, NULL, NULL, &timeout);
        if (sel == -1)
        {
            perror("select");
            exit(4);
        }
        if(sel == 0)
        {
            printf("\nselect timeout in %lds %dms\n\n", timeout.tv_sec, timeout.tv_usec);
            continue;
        }
        for (int i = 0; i < fd_max; i++)
        {
            if (FD_ISSET(i, &read_fds) == 0) continue;
            if (i == server_sock)
            {
                printf("server_sock is ready to read, got new connections\n");
                struct sockaddr_storage client_addr;
                memset(&client_addr, 0, sizeof client_addr);
                unsigned int client_addrlen = sizeof client_addr;
                // accept this connection
                int client_fd = accept(server_sock, (struct sockaddr *)(&client_addr), &client_addrlen);
                if (client_fd == -1)
                {
                    perror("accept");
                }
                else
                {
                    FD_SET(client_fd, &master);
                    if (client_fd > fd_max)
                    {
                        fd_max = client_fd;
                    }
                    // char remote_ip[INET6_ADDRSTRLEN];
                    // printf("selectserver: new connection from %s on socket %d\n",
                    //        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)(&client_addr)), remote_ip, INET6_ADDRSTRLEN),
                    //        client_fd);
                    print_sockaddr("selectserver: new connection: ", (struct sockaddr *)(&client_addr));
                }
            }
            else
            {
                int numbytes;
                char buf[256];
                int buf_size = sizeof buf;
                if ((numbytes == recv(i, buf, buf_size, 0)) <= 0)
                {
                    if (numbytes == 0)
                    {
                        printf("selectserver: socket %d hung up\n", i);
                    }
                    else
                    {
                        perror("recv");
                    }
                    close(i);
                    FD_CLR(i, &master);
                }
                else
                {
                    buf[numbytes] = '\0';
                    printf("recvd from socket:%d, len:%d, '%s'\n", i, numbytes, buf);
                    for (int j = 0; j < fd_max; j++)
                    {
                        if (FD_ISSET(j, &master) == 0) continue;
                        if (j == server_sock || j == i) continue;
                        int result = sendall(j, buf, numbytes);
                        if (result == -1)
                        {
                            perror("send");
                        }
                        else
                        {
                            printf("send to %d\n", j);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
