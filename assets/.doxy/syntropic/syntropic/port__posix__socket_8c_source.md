

# File port\_posix\_socket.c

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**posix**](dir_6d923334ab2bb155e44e019dd9da5e9c.md) **>** [**port\_posix\_socket.c**](port__posix__socket_8c.md)

[Go to the documentation of this file](port__posix__socket_8c.md)


```C++


#if !defined(ARDUINO) || (defined(ARDUINO_ARCH_ESP32) && \
                          (!defined(SYN_ARDUINO_HAS_ETHERNET) || (SYN_ARDUINO_HAS_ETHERNET == 0)))
#if defined(__unix__) || defined(__APPLE__) || defined(ESP_PLATFORM) || \
    defined(ARDUINO_ARCH_ESP32) || defined(__ZEPHYR__) || defined(_POSIX_VERSION)

#include "syntropic/port/syn_port_socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* ── Helpers ────────────────────────────────────────────────────────────── */

static struct sockaddr_in addr_to_sockaddr(const SYN_SockAddr *addr)
{
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if (addr != NULL) {
        sa.sin_port = htons(addr->port);
        memcpy(&sa.sin_addr.s_addr, addr->ip, 4U);
    }
    return sa;
}

static void posix_disable_sigpipe(int fd)
{
#ifdef SO_NOSIGPIPE
    int opt = 1;
    (void)setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
#else
    (void)fd;
#endif
}

static bool posix_poll_readable(SYN_Socket sock, uint32_t timeout_ms)
{
    if (sock < 0) {
        return false;
    }

    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int timeout_arg = (timeout_ms > 2147483647U) ? 2147483647 : (int)timeout_ms;
    int rc;
    do {
        rc = poll(&pfd, 1U, timeout_arg);
    } while (rc < 0 && errno == EINTR);

    return (rc > 0 && (pfd.revents & (POLLIN | POLLHUP | POLLERR)) != 0);
}

/* ── UDP ────────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_udp_open(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        return SYN_SOCKET_INVALID;
    }

    posix_disable_sigpipe(fd);

    if (port != 0U) {
        int opt = 1;
        (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len, const SYN_SockAddr *to)
{
    if (sock < 0 || to == NULL || (data == NULL && len > 0U)) {
        return -1;
    }
    struct sockaddr_in sa = addr_to_sockaddr(to);
    ssize_t n;
    do {
        n = sendto(sock, data, len, MSG_NOSIGNAL, (struct sockaddr *)&sa, sizeof(sa));
    } while (n < 0 && errno == EINTR);
    return (n < 0) ? -1 : (int)n;
}

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len, SYN_SockAddr *from,
                          uint32_t timeout_ms)
{
    if (sock < 0 || (buf == NULL && max_len > 0U)) {
        return -1;
    }

    int flags = 0;

    if (timeout_ms == 0U) {
        /* Non-blocking poll — SO_RCVTIMEO of {0,0} means infinite on
         * POSIX/lwIP, so use MSG_DONTWAIT instead. */
        flags = MSG_DONTWAIT;
    } else {
        struct timeval tv;
        tv.tv_sec = (time_t)(timeout_ms / 1000U);
        tv.tv_usec = (suseconds_t)((timeout_ms % 1000U) * 1000U);
        (void)setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t sa_len;
    ssize_t n;
    do {
        sa_len = sizeof(sa);
        n = recvfrom(sock, buf, max_len, flags, (struct sockaddr *)&sa, &sa_len);
    } while (n < 0 && errno == EINTR);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }

    if (n > 0 && from != NULL) {
        memcpy(from->ip, &sa.sin_addr.s_addr, 4U);
        from->port = ntohs(sa.sin_port);
    }

    return (int)n;
}

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip)
{
    if (sock < 0 || multicast_ip == NULL) {
        return SYN_INVALID_PARAM;
    }
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = INADDR_ANY;
    int res = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    return (res == 0) ? SYN_OK : SYN_ERROR;
}

bool syn_port_udp_readable(SYN_Socket sock)
{
    return posix_poll_readable(sock, 0U);
}

/* ── TCP ────────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr)
{
    if (addr == NULL) {
        return SYN_SOCKET_INVALID;
    }

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return SYN_SOCKET_INVALID;
    }

    posix_disable_sigpipe(fd);

    struct sockaddr_in sa = addr_to_sockaddr(addr);

    struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        close(fd);
        return SYN_SOCKET_INVALID;
    }

    return (SYN_Socket)fd;
}

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port)
{
    if (host == NULL) {
        return SYN_SOCKET_INVALID;
    }

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[6];
    (void)snprintf(port_str, sizeof(port_str), "%u", port);

    if (getaddrinfo(host, port_str, &hints, &res) != 0 || res == NULL) {
        return SYN_SOCKET_INVALID;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return SYN_SOCKET_INVALID;
    }

    posix_disable_sigpipe(fd);

    struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

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
    if (sock < 0 || (data == NULL && len > 0U)) {
        return -1;
    }
    ssize_t n;
    do {
        n = send(sock, data, len, MSG_NOSIGNAL);
    } while (n < 0 && errno == EINTR);
    return (n < 0) ? -1 : (int)n;
}

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len)
{
    if (sock < 0 || (data == NULL && len > 0U)) {
        return -1;
    }
    const uint8_t *p = (const uint8_t *)data;
    size_t sent = 0U;
    while (sent < len) {
        ssize_t n;
        do {
            n = send(sock, p + sent, len - sent, MSG_NOSIGNAL);
        } while (n < 0 && errno == EINTR);
        if (n <= 0) {
            return -1;
        }
        sent += (size_t)n;
    }
    return (int)sent;
}

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len, uint32_t timeout_ms)
{
    if (sock < 0 || (buf == NULL && max_len > 0U)) {
        return -1;
    }
    int flags = 0;
    if (timeout_ms == 0U) {
        flags = MSG_DONTWAIT;
    } else {
        struct timeval tv = {.tv_sec = (time_t)(timeout_ms / 1000U),
                             .tv_usec = (suseconds_t)((timeout_ms % 1000U) * 1000U)};
        (void)setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    ssize_t n;
    do {
        n = recv(sock, buf, max_len, flags);
    } while (n < 0 && errno == EINTR);
    if (n < 0) {
        return -1;
    }
    return (int)n;
}

bool syn_port_sock_readable(SYN_Socket sock)
{
    return posix_poll_readable(sock, 0U);
}

/* ── Common ─────────────────────────────────────────────────────────────── */

void syn_port_sock_close(SYN_Socket sock)
{
    if (sock >= 0) {
        close(sock);
    }
}

SYN_Socket syn_port_sock_listen(uint16_t port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return SYN_SOCKET_INVALID;
    }

    posix_disable_sigpipe(fd);

    int opt = 1;
    (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0 || fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
        close(fd);
        return SYN_SOCKET_INVALID;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
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
    if (!posix_poll_readable(listener, timeout_ms)) {
        return SYN_SOCKET_INVALID;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int fd;
    do {
        addr_len = sizeof(client_addr);
        fd = accept(listener, (struct sockaddr *)&client_addr, &addr_len);
    } while (fd < 0 && errno == EINTR);
    if (fd < 0) {
        /* EAGAIN / EWOULDBLOCK when no pending connection or peer reset */
        return SYN_SOCKET_INVALID;
    }

    int cfl = fcntl(fd, F_GETFL, 0);
    if (cfl >= 0 && (cfl & O_NONBLOCK) != 0) {
        (void)fcntl(fd, F_SETFL, cfl & ~O_NONBLOCK);
    }

    posix_disable_sigpipe(fd);

    struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    return (SYN_Socket)fd;
}

#endif /* POSIX platform guard */
#endif /* !ARDUINO || (ARDUINO_ARCH_ESP32 && !SYN_ARDUINO_HAS_ETHERNET) */
```


