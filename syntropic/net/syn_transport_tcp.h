/**
 * @file syn_transport_tcp.h
 * @brief Bridge for syn_transport over a TCP socket.
 * @ingroup syn_net
 */

#ifndef SYN_TRANSPORT_TCP_H
#define SYN_TRANSPORT_TCP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "syn_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TCP bridge transport layer context.
 */
typedef struct {
    SYN_Socket sock;                 /**< Embedded network TCP socket handle */
    uint8_t    state;                /**< Parser state: 0=len MSB, 1=len LSB, 2=payload */
    uint16_t   payload_len;          /**< Expected size of the incoming frame payload */
    uint16_t   bytes_read;           /**< Bytes of current payload read so far */
    uint8_t    rx_buf[128];          /**< Buffer for staging incoming packet payloads */
} SYN_TransportTcp;

/**
 * @brief Initialize a TCP transport bridge.
 *
 * Configures the transport interface vtable with the socket-specific operations.
 *
 * @param t    Pointer to the parent transport structure to configure.
 * @param tcp  Pointer to the TCP transport context structure to associate.
 * @param sock Connected TCP socket to bind to this transport.
 */
void syn_transport_tcp_init(SYN_Transport *t, SYN_TransportTcp *tcp, SYN_Socket sock);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRANSPORT_TCP_H */
