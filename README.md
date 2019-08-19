# socket相关的结构体
1. struct addrinfo</br>
a more recent invention, and is used to prep the socket address structures for subsequent use. It's also used in host name lookups, and service name lookups.</br>
一个较新的结构体， 用来表示socket地址
```
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next;      // linked list, next node
};
```
2. struct sockaddr</br>
holds socket address information for many types of sockets</br>
表示各种类型的socket地址信息
```
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address, a destination address and port number
};
```
3. struct sockaddr_in & in_addr</br>
To deal with struct sockaddr, programmers created a parallel structure: struct sockaddr_in ("in" for "Internet") to be used with IPv4.</br>
用于表示IPv4网络地址，sockaddr 和 sockaddr_in 长度一样，可相互转换
```
// (IPv4 only--see struct sockaddr_in6 for IPv6)
struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
```
```
// (IPv4 only--see struct in6_addr for IPv6)
// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};
```
4. sockaddr_in6 and in6_addr</br>
same as struct sockaddr_in & in_addr, but for IPv6
```
// (IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)
struct sockaddr_in6 {
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
};
```
```
struct in6_addr {
    unsigned char   s6_addr[16];   // IPv6 address
};
```
5. sockaddr_storage</br>
another simple structure, struct sockaddr_storage that is designed to be large enough to hold both IPv4 and IPv6 structures.
```
struct sockaddr_storage {
    sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];IPv4
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};
```
参考：https://blog.csdn.net/fanx021/article/details/80549879</br>
sockaddr 常用于 bind、connect、recvfrom、sendto 等函数的参数，指明地址信息，是一种通用的套接字地址。</br>
sockaddr_in 是 internet 环境下套接字的地址形式。所以在网络编程中我们会对 sockaddr_in 结构体进行操作，使用 sockaddr_in 来建立所需的信息，最后使用类型转化就可以了。一般先把 sockaddr_in 变量赋值后，强制类型转换后传入</br>
用 sockaddr 做参数的函数：sockaddr_in 用于 socket 定义和赋值；sockaddr 用于函数参数。

# socket 接口
1. getaddrinfo()</br>
DNS解析，替换旧的 gethostbyname，以支持IPv4 & IPv6
2. socket()</br>
获取socket fd:
```
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```
3. bind()</br>
把获取到的socket绑定到指定的端口
```
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```
4. connect()</br>
通过指定的socket连接到指定的地址
```
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
```
5. listen()</br>
```
int listen(int sockfd, int backlog);
```
6. accept()</br>
```
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

for a simple server:
```
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

int main(void)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // make a socket, bind it, and listen on it:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    // now accept an incoming connection:

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    // ready to communicate on socket descriptor new_fd!
    .
    .
    .
}
```
7. 读写socket</br>
recv() send(): 读写stream socket</br>
sendto() recvfrom(): 读写datagram socket</br>
```
int send(int sockfd, const void *msg, int len, int flags);
int recv(int sockfd, void *buf, int len, int flags);
int sendto(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, socklen_t tolen);
int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);
```
8. close/shutdown</br>
```
close(sockfd); // Anyone attempting to read or write the socket on the remote end will receive an error.
shutdown(int sockfd, int how);
how:
  0 Further receives are disallowed
  1 Further sends are disallowed
  2 Further sends and receives are disallowed (like close())
```
