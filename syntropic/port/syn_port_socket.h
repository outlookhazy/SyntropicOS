/**
 * @file syn_port_socket.h
 * @brief TCP socket port interface — implement these for your platform.
 *
 * Minimal BSD-like socket abstraction for TCP client connections.
 * Higher-level modules (HTTP client, MQTT, OTA) use this interface
 * without knowing the underlying TCP/IP stack.
 *
 * Implement these functions by wrapping your platform's socket API:
 *   - lwIP (raw or socket mode)
 *   - ESP-IDF (POSIX sockets via lwIP)
 *   - POSIX (Linux/macOS for development)
 *   - Vendor TCP/IP stack
 *
 * @par Usage
 * @code
 *   SYN_Socket sock = syn_port_sock_connect_host("example.com", 80);
 *   if (sock < 0) { handle error }
 *
 *   syn_port_sock_send(sock, "GET / HTTP/1.1\r\n\r\n", 18);
 *
 *   uint8_t buf[256];
 *   int n = syn_port_sock_recv(sock, buf, sizeof(buf), 5000);
 *
 *   syn_port_sock_close(sock);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_PORT_SOCKET_H
#define SYN_PORT_SOCKET_H

#include <stdint.h>
#include <stddef.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Socket handle. Negative values indicate error / invalid. */
typedef int SYN_Socket;

/** @brief Sentinel value for an invalid/uninitialized socket. */
#define SYN_SOCKET_INVALID  (-1)

/** IPv4 address + port. */
typedef struct {
    uint8_t  ip[4];       /**< IPv4 address bytes (network order)       */
    uint16_t port;        /**< Port number (host order)                  */
} SYN_SockAddr;

/* ── Connection ─────────────────────────────────────────────────────────── */

/**
 * @brief Connect to a remote host by IP address.
 *
 * Opens a TCP connection. Blocking.
 *
 * @param addr  Remote address (IPv4 + port).
 * @return Socket handle on success, SYN_SOCKET_INVALID on failure.
 */
SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr);

/**
 * @brief Connect to a remote host by hostname.
 *
 * Performs DNS resolution and opens a TCP connection. Blocking.
 *
 * @param host  Hostname or dotted-decimal IP string.
 * @param port  Port number.
 * @return Socket handle on success, SYN_SOCKET_INVALID on failure.
 */
SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port);

/* ── Data transfer ──────────────────────────────────────────────────────── */

/**
 * @brief Send data over a connected socket.
 *
 * May send fewer bytes than requested (partial send).
 *
 * @param sock  Socket handle.
 * @param data  Data to send.
 * @param len   Number of bytes to send.
 * @return Number of bytes sent, or -1 on error.
 */
int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len);

/**
 * @brief Send all data over a connected socket.
 *
 * Loops internally until all bytes are sent or an error occurs.
 *
 * @param sock  Socket handle.
 * @param data  Data to send.
 * @param len   Number of bytes to send.
 * @return Number of bytes sent (== len on success), or -1 on error.
 */
int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len);

/**
 * @brief Receive data from a connected socket.
 *
 * Returns available data up to max_len. Blocks up to timeout_ms.
 *
 * @param sock       Socket handle.
 * @param buf        Receive buffer.
 * @param max_len    Buffer capacity.
 * @param timeout_ms Timeout in milliseconds (0 = non-blocking).
 * @return Number of bytes received, 0 if connection closed, -1 on error/timeout.
 */
int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                       uint32_t timeout_ms);

/* ── Server ─────────────────────────────────────────────────────────────── */

/**
 * @brief Create a listening TCP socket on the given port.
 *
 * Binds to all interfaces and begins listening for connections.
 *
 * @param port     Port number to listen on.
 * @param backlog  Maximum pending connections (typically 1–5).
 * @return Listener socket handle, or SYN_SOCKET_INVALID on failure.
 */
SYN_Socket syn_port_sock_listen(uint16_t port, int backlog);

/**
 * @brief Accept an incoming connection on a listener socket.
 *
 * Blocks until a connection arrives or the timeout expires.
 *
 * @param listener   Listener socket from syn_port_sock_listen().
 * @param timeout_ms Timeout in milliseconds (0 = block indefinitely).
 * @return Connected socket handle, or SYN_SOCKET_INVALID on timeout/error.
 */
SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms);

/* ── UDP ────────────────────────────────────────────────────────────────── */

/**
 * @brief Open a UDP socket bound to the given port.
 *
 * @param port  Local port to bind to, or 0 for ephemeral.
 * @return Socket handle on success, SYN_SOCKET_INVALID on failure.
 */
SYN_Socket syn_port_udp_open(uint16_t port);

/**
 * @brief Send a UDP packet to a destination address.
 *
 * @param sock  Socket handle.
 * @param data  Payload to send.
 * @param len   Payload length.
 * @param to    Destination address (IP + port).
 * @return Number of bytes sent, or -1 on error.
 */
int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                        const SYN_SockAddr *to);

/**
 * @brief Receive a UDP packet.
 *
 * @param sock        Socket handle.
 * @param buf         Receive buffer.
 * @param max_len     Buffer capacity.
 * @param from        [out] Source address.
 * @param timeout_ms  Receive timeout in milliseconds.
 *                    **If 0, must return immediately (non-blocking poll).**
 *                    Cooperative tasks (protothreads) rely on this to avoid
 *                    blocking the scheduler. Port implementations MUST NOT
 *                    treat 0 as "block forever."
 * @return Number of bytes received, 0 if no data available (non-blocking),
 *         or -1 on error.
 */
int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                          SYN_SockAddr *from, uint32_t timeout_ms);

/**
 * @brief Join a multicast group.
 *
 * @param sock          UDP socket handle.
 * @param multicast_ip  Multicast IP string (e.g. "224.0.0.251").
 * @return SYN_OK on success, SYN_ERROR on failure.
 */
SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip);



/* ── Lifecycle ──────────────────────────────────────────────────────────── */



/**
 * @brief Close a socket and release resources.
 *
 * @param sock  Socket handle to close.
 */
void syn_port_sock_close(SYN_Socket sock);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SOCKET_H */
