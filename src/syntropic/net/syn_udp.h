/**
 * @file syn_udp.h
 * @brief Zero-Alloc Native UDP Demuxing Engine & Task-Blocking Layer.
 * @ingroup syn_net
 */

#ifndef SYN_UDP_H
#define SYN_UDP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ─────────────────────────────────────────────────────────── */

#ifndef SYN_UDP_MAX_SOCKETS
#define SYN_UDP_MAX_SOCKETS 4 /**< Maximum concurrent UDP bound sockets */
#endif

#ifndef SYN_UDP_BUF_SIZE
#define SYN_UDP_BUF_SIZE 256 /**< UDP socket internal RX buffer size in bytes */
#endif

#ifndef SYN_UDP_MAX_DGRAMS
#define SYN_UDP_MAX_DGRAMS 8U /**< Maximum queued UDP datagram descriptors per socket */
#endif

/** @brief UDP packet header structure (8 bytes). */
typedef struct {
    uint16_t src_port; /**< Source port number. */
    uint16_t dst_port; /**< Destination port number. */
    uint16_t length;   /**< Length of UDP header + payload in bytes. */
    uint16_t checksum; /**< Checksum over pseudo-header and payload. */
} SYN_UdpHeader;

/** @brief Queued UDP datagram metadata descriptor preserving packet boundaries. */
typedef struct {
    uint32_t src_ip;   /**< Sender IPv4 address. */
    uint16_t src_port; /**< Sender UDP port. */
    uint16_t len;      /**< Datagram payload byte length. */
} SYN_UdpDatagram;

/* ── UDP Socket Slot ────────────────────────────────────────────────────── */

/** @brief UDP socket slot control block. */
typedef struct {
    bool is_bound;        /**< True if socket is bound to a local port. */
    uint16_t local_port;  /**< Bound local port number. */
    uint32_t remote_ip;   /**< Sender IPv4 address of front queued datagram. */
    uint16_t remote_port; /**< Sender UDP port of front queued datagram. */

    uint8_t rx_buf[SYN_UDP_BUF_SIZE];           /**< Queued receive payload byte buffer. */
    uint16_t rx_len;                            /**< Front datagram payload byte length. */
    uint16_t rx_total_len;                      /**< Total payload bytes across queued datagrams. */
    SYN_UdpDatagram dgrams[SYN_UDP_MAX_DGRAMS]; /**< Per-datagram boundary ring buffer. */
    uint8_t dgram_head;                         /**< Head index in datagram ring. */
    uint8_t dgram_count;                        /**< Number of queued datagrams. */

    SYN_Task *blocked_task; /**< Protothread task sleeping on this UDP socket. */
} SYN_UdpSocket;

/* ── UDP Stack Container ───────────────────────────────────────────────── */

/** @brief UDP protocol stack container instance. */
typedef struct {
    SYN_ETH *eth;                               /**< Associated Ethernet interface instance. */
    SYN_UdpSocket sockets[SYN_UDP_MAX_SOCKETS]; /**< Array of managed UDP sockets. */
    uint16_t next_ephemeral_port;               /**< Ephemeral local port cursor (49152..65535). */
} SYN_UDP;

/* ── Functions ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize UDP Stack.
 * @param udp Pointer to UDP stack instance.
 * @param eth Pointer to Ethernet interface instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_udp_init(SYN_UDP *udp, SYN_ETH *eth);

/**
 * @brief Bind UDP socket to local port.
 * @param udp        Pointer to UDP stack instance.
 * @param local_port Port number to bind.
 * @return Pointer to allocated UDP socket slot, or NULL if capacity exceeded.
 */
SYN_UdpSocket *syn_udp_bind(SYN_UDP *udp, uint16_t local_port);

/**
 * @brief Unbind and release a UDP socket slot.
 * @param sock Pointer to UDP socket slot.
 */
void syn_udp_unbind(SYN_UdpSocket *sock);

/**
 * @brief Process incoming UDP IPv4 packet.
 * @param udp       Pointer to UDP stack instance.
 * @param ip_packet Pointer to raw IPv4 packet.
 * @param len       IPv4 packet byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_udp_process_packet(SYN_UDP *udp, const uint8_t *ip_packet, size_t len);

/**
 * @brief Construct outbound UDP packet into frame output buffer.
 * @param udp      Pointer to UDP stack.
 * @param src_port Source UDP port.
 * @param dst_ip   Destination IPv4 address.
 * @param dst_port Destination UDP port.
 * @param data     Pointer to payload data buffer.
 * @param data_len Payload byte length.
 * @param tx_out   Output Ethernet frame buffer.
 * @param tx_len   Pointer to receive final Ethernet frame length.
 * @return SYN_OK on success.
 */
int syn_udp_sendto(SYN_UDP *udp, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                   const uint8_t *data, size_t data_len, uint8_t *tx_out, size_t *tx_len);

/**
 * @brief Compute UDP 16-bit 1's complement checksum over pseudo-header and segment.
 * @param src_ip  Source 32-bit IPv4 address.
 * @param dst_ip  Destination 32-bit IPv4 address.
 * @param udp_seg Pointer to raw UDP segment bytes.
 * @param len     UDP segment length in bytes.
 * @return Computed 16-bit UDP checksum.
 */
uint16_t syn_udp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *udp_seg, size_t len);

/**
 * @brief Dequeue a single UDP datagram from a bound socket, preserving datagram boundaries.
 * @param sock       Pointer to UDP socket slot.
 * @param buf        Destination buffer for datagram payload.
 * @param max_len    Maximum bytes to copy into buf.
 * @param bytes_read [out] Bytes copied into buf (0 if no datagram queued).
 * @param src_ip     [out] Optional pointer receiving sender IPv4 address.
 * @param src_port   [out] Optional pointer receiving sender UDP port.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL required pointer.
 */
SYN_Status syn_udp_recv(SYN_UdpSocket *sock, void *buf, size_t max_len, size_t *bytes_read,
                        uint32_t *src_ip, uint16_t *src_port);

/**
 * @brief Validate pointer arguments for PT_UDP_BLOCK_READ without triggering -Waddress.
 * @param pt         Protothread state pointer.
 * @param task       Task instance pointer.
 * @param sock       UDP socket pointer.
 * @param buf        Destination read buffer pointer.
 * @param bytes_read Output byte count pointer.
 * @return true if all required pointers are non-NULL, false otherwise.
 */
static inline bool syn_udp_pt_args_valid(SYN_PT *pt, const SYN_Task *task,
                                         const SYN_UdpSocket *sock, const void *buf,
                                         const size_t *bytes_read)
{
    if (pt == NULL || task == NULL || sock == NULL || buf == NULL || bytes_read == NULL) {
        if (pt != NULL) {
            PT_INIT(pt);
        }
        return false;
    }
    return true;
}

/* ── Task Blocking Macros ───────────────────────────────────────────────── */

/**
 * @brief Protothread helper macro to block task until UDP packet arrives.
 * @param pt         Protothread state.
 * @param task       Task instance.
 * @param sock       UDP socket pointer.
 * @param buf        Destination read buffer.
 * @param max_len    Maximum bytes to read.
 * @param bytes_read Pointer to output size_t receiving count of read bytes.
 * @param src_ip     Pointer to uint32_t receiving sender IP address.
 * @param src_port   Pointer to uint16_t receiving sender port.
 */
#define PT_UDP_BLOCK_READ(pt, task, sock, buf, max_len, bytes_read, src_ip, src_port)     \
    do {                                                                                  \
        while (1) {                                                                       \
            if (!syn_udp_pt_args_valid((pt), (task), (sock), (buf), (bytes_read))) {      \
                return PT_EXITED;                                                         \
            }                                                                             \
            (sock)->blocked_task = (task);                                                \
            syn_port_enter_critical();                                                    \
            if ((task)->state == (uint8_t)SYN_TASK_READY) {                               \
                (task)->state = (uint8_t)SYN_TASK_BLOCKED;                                \
                SYN_PT_STATE_BARRIER();                                                   \
            }                                                                             \
            syn_port_exit_critical();                                                     \
            if ((sock)->dgram_count > 0U || (sock)->rx_len > 0U || !(sock)->is_bound) {   \
                syn_port_enter_critical();                                                \
                if ((task)->state == (uint8_t)SYN_TASK_BLOCKED) {                         \
                    (task)->state = (uint8_t)SYN_TASK_READY;                              \
                }                                                                         \
                syn_port_exit_critical();                                                 \
                break;                                                                    \
            }                                                                             \
            PT_PASS(pt);                                                                  \
        }                                                                                 \
        (sock)->blocked_task = NULL;                                                      \
        (void)syn_udp_recv((sock), (buf), (max_len), (bytes_read), (src_ip), (src_port)); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDP_H */
