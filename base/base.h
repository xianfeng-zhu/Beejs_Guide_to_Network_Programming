#ifndef _util_h
#define _util_h

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  else
  {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

in_port_t get_in_port_ntohs(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
    return ntohs(((struct sockaddr_in *)sa)->sin_port);
  else
    return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

void print_sockaddr(char * msg, struct sockaddr *sa)
{
  void *addr;
  char *ip_ver;
  if (sa->sa_family == AF_INET)
  {
    ip_ver = "IPV4";
    struct sockaddr_in *addrv4 = (struct sockaddr_in *)sa;
    addr = &(addrv4->sin_addr);
  }
  else
  {
    ip_ver = "IPV6";
    struct sockaddr_in6 *addrv6 = (struct sockaddr_in6 *)sa;
    addr = &(addrv6->sin6_addr);
  }
  char ipstr[INET6_ADDRSTRLEN];
  inet_ntop(sa->sa_family, addr, ipstr, sizeof ipstr);
  printf("%s%s: '%s'-%d\n", msg, ip_ver, ipstr, get_in_port_ntohs(sa));
}

void print_addrinfo(struct addrinfo *p)
{
  char ipstr[INET6_ADDRSTRLEN];
  for (; p != NULL; p = p->ai_next)
  {
    print_sockaddr("  ", p->ai_addr);
  }
}

int nslookup(const char *hostname)
{
  int status;
  struct addrinfo hints;
  struct addrinfo *serv_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(hostname, NULL, &hints, &serv_info)) != 0)
  {
    fprintf(stderr, "getaddrinfo error: %d: %s\n", status, gai_strerror(status));
    return 2;
  }
  printf("IP address for %s:\n", hostname);
  print_addrinfo(serv_info);

  freeaddrinfo(serv_info);
  return 0;
}

int sendall(int socket, char * buf, int count)
{
  int totalsent = 0;
  int bytesleft = count;
  int n;
  while (totalsent < count)
  {
    n = send(socket, buf + totalsent, bytesleft, 0);
    if (n == -1) break;
    totalsent += n;
    bytesleft -= n;
  }
  return n == -1 ? -1 : totalsent;
}

#endif
