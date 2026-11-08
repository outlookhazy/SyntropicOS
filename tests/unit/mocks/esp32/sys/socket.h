#ifndef MOCK_SYS_SOCKET_H
#define MOCK_SYS_SOCKET_H

#include <stddef.h>
#include <stdint.h>
#include <sys/select.h>

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOL_SOCKET 1
#define SO_REUSEADDR 2
#define SO_RCVTIMEO 20
#define IPPROTO_IP 0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define INADDR_ANY 0UL
#define IP_ADD_MEMBERSHIP 35
#define MSG_DONTWAIT 0x40

typedef int socklen_t;

struct in_addr {
    uint32_t s_addr;
};
struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
};
struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
};
struct ip_mreq {
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
};

static inline int socket(int d, int t, int p)
{
    (void)d;
    (void)t;
    (void)p;
    return 1;
}
static inline int setsockopt(int s, int l, int n, const void *v, socklen_t o)
{
    (void)s;
    (void)l;
    (void)n;
    (void)v;
    (void)o;
    return 0;
}
static inline int bind(int s, const struct sockaddr *a, socklen_t l)
{
    (void)s;
    (void)a;
    (void)l;
    return 0;
}
static inline int listen(int s, int b)
{
    (void)s;
    (void)b;
    return 0;
}
static inline int accept(int s, struct sockaddr *a, socklen_t *l)
{
    (void)s;
    (void)a;
    (void)l;
    return -1;
}
static inline int connect(int s, const struct sockaddr *a, socklen_t l)
{
    (void)s;
    (void)a;
    (void)l;
    return 0;
}
static inline int send(int s, const void *b, size_t l, int f)
{
    (void)s;
    (void)b;
    (void)l;
    (void)f;
    return (int)l;
}
static inline int recv(int s, void *b, size_t l, int f)
{
    (void)s;
    (void)b;
    (void)l;
    (void)f;
    return 0;
}
static inline int sendto(int s, const void *b, size_t l, int f, const struct sockaddr *a,
                         socklen_t al)
{
    (void)s;
    (void)b;
    (void)l;
    (void)f;
    (void)a;
    (void)al;
    return (int)l;
}
static inline int recvfrom(int s, void *b, size_t l, int f, struct sockaddr *a, socklen_t *al)
{
    (void)s;
    (void)b;
    (void)l;
    (void)f;
    (void)a;
    (void)al;
    return 0;
}
static inline int close(int s)
{
    (void)s;
    return 0;
}

#endif
