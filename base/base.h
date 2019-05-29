#ifndef _util_h
#define _util_h

#include <stdio.h>
#include <string.h>
#include <netdb.h>

void * get_in_addr(struct sockaddr * sa)
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

in_port_t get_in_port_ntohs(struct sockaddr * sa)
{
    if (sa->sa_family == AF_INET) 
      return ntohs(((struct sockaddr_in *)sa)->sin_port);
    else 
      return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

void print_addrinfo(struct addrinfo * p)
{
  char ipstr[INET6_ADDRSTRLEN];
  for (; p != NULL; p = p->ai_next)
  {
    void * addr;
    char * ip_ver;
    if (p->ai_family == AF_INET)
    {
      ip_ver = "IPV4";
      struct sockaddr_in *addrv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(addrv4->sin_addr);
    }
    else
    {
      ip_ver = "IPV6";
      struct sockaddr_in6 *addrv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(addrv6->sin6_addr);
    }
    // memset(ipstr, 0, sizeof ipstr);
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf("  %s: '%s'-%d\n", ip_ver, ipstr, get_in_port_ntohs(p->ai_addr));
  }
}

int nslookup(const char * hostname)
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
#endif
