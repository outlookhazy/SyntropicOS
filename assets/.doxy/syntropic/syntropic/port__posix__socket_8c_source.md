

# File port\_posix\_socket.c

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**posix**](dir_6d923334ab2bb155e44e019dd9da5e9c.md) **>** [**port\_posix\_socket.c**](port__posix__socket_8c.md)

[Go to the documentation of this file](port__posix__socket_8c.md)


```C++

#if defined(__unix__) || defined(__APPLE__) || defined(ESP_PLATFORM) || \
    defined(__ZEPHYR__) || defined(_POSIX_VERSION)

#include "syntropic/port/syn_port_socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static struct sockaddr_in addr_to_sockaddr(const SYN_SockAddr *addr)
{
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(addr->port);
    memcpy(&sa.sin_addr.s_addr, addr->ip, 4);
    return sa;
}

/* ── UDP ────────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_udp_open(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) return SYN_SOCKET_INVALID;

    if (port != 0) {
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(port);
        bind_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            close(fd);
            return SYN_SOCKET_INVALID;
        }
    }

    return (SYN_Socket)fd;
}

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                        const SYN_SockAddr *to)
{
    struct sockaddr_in sa = addr_to_sockaddr(to);
    return sendto(sock, data, len, 0, (struct sockaddr *)&sa, sizeof(sa));
}

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                          SYN_SockAddr *from, uint32_t timeout_ms)
{
    int flags = 0;

    if (timeout_ms == 0) {
        /* Non-blocking poll — SO_RCVTIMEO of {0,0} means infinite on
         * POSIX/lwIP, so use MSG_DONTWAIT instead. */
        flags = MSG_DONTWAIT;
    } else {
        struct timeval tv;
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t sa_len = sizeof(sa);
    int n = recvfrom(sock, buf, max_len, flags, (struct sockaddr *)&sa, &sa_len);

    if (n > 0 && from) {
        memcpy(from->ip, &sa.sin_addr.s_addr, 4);
        from->port = ntohs(sa.sin_port);
    }

    return n;
}

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip)
{
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        return SYN_ERROR;
    return SYN_OK;
}

/* ── TCP ────────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return SYN_SOCKET_INVALID;

    struct sockaddr_in sa = addr_to_sockaddr(addr);

    struct timeval tv = { .tv_sec = 10, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        close(fd);
        return SYN_SOCKET_INVALID;
    }

    return (SYN_Socket)fd;
}

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port)
{
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", port);

    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res)
        return SYN_SOCKET_INVALID;

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return SYN_SOCKET_INVALID;
    }

    struct timeval tv = { .tv_sec = 10, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        close(fd);
        freeaddrinfo(res);
        return SYN_SOCKET_INVALID;
    }

    freeaddrinfo(res);
    return (SYN_Socket)fd;
}

int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len)
{
    return send(sock, data, len, 0);
}

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t sent = 0;
    while (sent < len) {
        int n = send(sock, p + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return (int)sent;
}

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                       uint32_t timeout_ms)
{
    struct timeval tv = {
        .tv_sec  = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };
    if (timeout_ms == 0) {
        tv.tv_sec  = 0;
        tv.tv_usec = 0;
    }
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    int n = recv(sock, buf, max_len, 0);
    if (n < 0) return -1;
    return n;
}

/* ── Common ─────────────────────────────────────────────────────────────── */

void syn_port_sock_close(SYN_Socket sock)
{
    if (sock >= 0) close(sock);
}

SYN_Socket syn_port_sock_listen(uint16_t port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return SYN_SOCKET_INVALID;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return SYN_SOCKET_INVALID;
    }

    if (listen(fd, backlog) < 0) {
        close(fd);
        return SYN_SOCKET_INVALID;
    }

    return (SYN_Socket)fd;
}

SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms)
{
    struct timeval tv = {
        .tv_sec  = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };
    setsockopt(listener, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int fd = accept(listener, (struct sockaddr *)&client_addr, &addr_len);
    if (fd < 0) return SYN_SOCKET_INVALID;
    return (SYN_Socket)fd;
}

#endif /* POSIX platform guard */
```


