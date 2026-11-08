/**
 * @file syn_transport_udp.h
 * @brief Bridge binding syn_port_udp_* API to native syn_udp stack.
 * @ingroup syn_net
 */

#ifndef SYN_TRANSPORT_UDP_H
#define SYN_TRANSPORT_UDP_H

#include "syntropic/common/syn_defs.h"

#if !defined(SYN_USE_TRANSPORT_UDP) || SYN_USE_TRANSPORT_UDP

#include "syntropic/net/syn_udp.h"
#include "syntropic/port/syn_port_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ethernet frame transmit callback signature for the native UDP bridge.
 * @param frame Pointer to Ethernet frame bytes.
 * @param len   Ethernet frame byte length.
 * @return SYN_OK on success, error status otherwise.
 */
typedef SYN_Status (*SYN_TransportEthTxFn)(const void *frame, size_t len);

/**
 * @brief Bind native syn_udp instance to the syn_port_udp_* socket interface.
 *
 * @param udp Pointer to native SYN_UDP stack instance.
 */
void syn_transport_udp_set_instance(SYN_UDP *udp);

/**
 * @brief Override the Ethernet transmit function used by syn_transport_udp_sendto.
 * @param tx_fn Custom transmit callback, or NULL to restore default syn_port_eth_tx.
 */
void syn_transport_udp_set_eth_tx(SYN_TransportEthTxFn tx_fn);

/**
 * @brief Get the active native SYN_UDP stack instance.
 * @return Pointer to SYN_UDP instance, or NULL.
 */
SYN_UDP *syn_transport_udp_get_instance(void);

/**
 * @brief Open and bind a native UDP socket handle (assigning ephemeral port when @p port is 0).
 * @param port Local UDP port, or 0 for dynamic ephemeral port (49152..65535).
 * @return Socket handle (>= 100), or SYN_SOCKET_INVALID on failure.
 */
SYN_Socket syn_transport_udp_open(uint16_t port);

/**
 * @brief Transmit a UDP datagram via the bound native UDP instance and Ethernet port.
 * @param sock_handle Native UDP socket handle.
 * @param data        Datagram payload bytes.
 * @param len         Payload byte length.
 * @param to          Destination IPv4 endpoint.
 * @return Number of payload bytes transmitted, or -1 on error.
 */
int syn_transport_udp_sendto(SYN_Socket sock_handle, const void *data, size_t len,
                             const SYN_SockAddr *to);

/**
 * @brief Non-blocking receive from a bound native UDP socket handle.
 * @param sock_handle Native UDP socket handle.
 * @param buf         [out] Destination buffer.
 * @param max_len     Destination buffer capacity.
 * @param from        [out] Optional sender endpoint.
 * @param timeout_ms  Unused in non-blocking native bridge (0).
 * @return Bytes copied (> 0), 0 if no datagram pending, or -1 on invalid handle/buffer.
 */
int syn_transport_udp_recvfrom(SYN_Socket sock_handle, void *buf, size_t max_len,
                               SYN_SockAddr *from, uint32_t timeout_ms);

/**
 * @brief Check whether a bound native UDP socket has queued receive bytes.
 * @param sock_handle Native UDP socket handle.
 * @return true if unread UDP bytes are queued, false otherwise.
 */
bool syn_transport_udp_readable(SYN_Socket sock_handle);

/**
 * @brief Close and unbind a native UDP socket handle.
 * @param sock_handle Native UDP socket handle.
 */
void syn_transport_udp_close(SYN_Socket sock_handle);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_TRANSPORT_UDP */

#endif /* SYN_TRANSPORT_UDP_H */
