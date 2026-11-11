/**
 * @file syn_tcp.h
 * @brief Zero-Alloc Native TCP State Machine & Segment Engine.
 * @ingroup syn_net
 */

#ifndef SYN_TCP_H
#define SYN_TCP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ─────────────────────────────────────────────────────────── */

#ifndef SYN_TCP_MAX_CONNS
#define SYN_TCP_MAX_CONNS 1 /**< Maximum concurrent TCP connections */
#endif

#ifndef SYN_TCP_BUF_SIZE
#define SYN_TCP_BUF_SIZE 512 /**< TCP RX/TX internal payload buffer size in bytes */
#endif

#ifndef SYN_TCP_SYN_TIMEOUT_MS
#define SYN_TCP_SYN_TIMEOUT_MS 3000U /**< Half-open SYN_RCVD slot reclamation timeout in ms */
#endif

/** TCP Header Flags */
#define SYN_TCP_FLAG_FIN 0x01 /**< Finish flag: No more data from sender */
#define SYN_TCP_FLAG_SYN 0x02 /**< Synchronize sequence numbers flag */
#define SYN_TCP_FLAG_RST 0x04 /**< Reset connection flag */
#define SYN_TCP_FLAG_PSH 0x08 /**< Push function flag */
#define SYN_TCP_FLAG_ACK 0x10 /**< Acknowledgment field significant flag */

/* ── States ────────────────────────────────────────────────────────────── */

/** TCP Connection State Machine Enum. */
typedef enum {
    SYN_TCP_CLOSED = 0,
    SYN_TCP_LISTEN = 1,
    SYN_TCP_SYN_RCVD = 2,
    SYN_TCP_ESTABLISHED = 3,
    SYN_TCP_FIN_WAIT = 4,
    SYN_TCP_CLOSE_WAIT = 5,
} SYN_TcpState;

/* ── TCP Header Format ─────────────────────────────────────────────────── */

/** @brief TCP packet header structure. */
typedef struct {
    uint16_t src_port;   /**< Source port number. */
    uint16_t dst_port;   /**< Destination port number. */
    uint32_t seq_num;    /**< Sequence number. */
    uint32_t ack_num;    /**< Acknowledgment number. */
    uint8_t data_offset; /**< Header length in 32-bit words (top 4 bits). */
    uint8_t flags;       /**< Control flags (SYN, ACK, FIN, RST, PSH, URG). */
    uint16_t window;     /**< Receive window size. */
    uint16_t checksum;   /**< Checksum value over pseudo-header and payload. */
    uint16_t urgent_ptr; /**< Urgent pointer. */
} SYN_TcpHeader;

/* ── TCP Connection Block ──────────────────────────────────────────────── */

/** @brief Single TCP socket connection control block. */
typedef struct SYN_TcpConn {
    SYN_TcpState state;    /**< Connection state machine state. */
    uint32_t local_ip;     /**< Local IPv4 address. */
    uint32_t remote_ip;    /**< Remote peer IPv4 address. */
    uint16_t local_port;   /**< Local port. */
    uint16_t remote_port;  /**< Remote peer port. */
    uint8_t remote_mac[6]; /**< Remote peer Ethernet MAC address. */

    uint32_t seq_nxt;     /**< Next sequence number to send. */
    uint32_t ack_nxt;     /**< Expected next sequence number to receive. */
    uint32_t syn_rcvd_ms; /**< Tick timestamp when connection entered SYN_TCP_SYN_RCVD. */

    uint8_t rx_buf[SYN_TCP_BUF_SIZE]; /**< Receive data buffer. */
    uint16_t rx_len;                  /**< Length of buffered receive bytes. */

    uint8_t tx_buf[SYN_TCP_BUF_SIZE]; /**< Transmit data buffer. */
    uint16_t tx_len;                  /**< Length of buffered transmit bytes. */

    SYN_Task *blocked_task; /**< Task currently blocked waiting on TCP data. */
} SYN_TcpConn;

/* ── TCP Stack Container ───────────────────────────────────────────────── */

/** @brief Top-level TCP stack instance container. */
typedef struct SYN_TCP {
    SYN_ETH *eth;                             /**< Associated Ethernet interface instance. */
    SYN_TcpConn conns[SYN_TCP_MAX_CONNS];     /**< Array of managed TCP connections. */
    uint16_t listen_ports[SYN_TCP_MAX_CONNS]; /**< Per-slot passive listener ports. */
    bool listening;                           /**< True if passive listener is active. */
    uint16_t listen_port; /**< Most recently configured passive listener port. */
} SYN_TCP;

/* ── Functions ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize TCP Stack.
 * @param tcp Pointer to TCP stack container.
 * @param eth Pointer to Ethernet interface engine instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_tcp_init(SYN_TCP *tcp, SYN_ETH *eth);

/**
 * @brief Bind TCP listener to port.
 * @param tcp  Pointer to TCP stack container.
 * @param port Port number to listen on.
 * @return SYN_OK on success.
 */
SYN_Status syn_tcp_listen(SYN_TCP *tcp, uint16_t port);

/**
 * @brief Close a TCP connection and re-arm passive listener if configured.
 * @param tcp  Pointer to TCP stack container.
 * @param conn Pointer to TCP connection slot.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL pointer.
 */
SYN_Status syn_tcp_close(SYN_TCP *tcp, SYN_TcpConn *conn);

/**
 * @brief Process incoming TCP IPv4 segment.
 * @param tcp       Pointer to TCP stack container.
 * @param ip_packet Pointer to raw Ethernet II + IPv4 + TCP frame.
 * @param len       Frame byte length (>= 54 bytes).
 * @param tx_out    Output buffer for immediate TCP reply/RST frame (at least 54 bytes).
 * @param tx_len    Pointer to receive output frame byte length (0 if none generated).
 * @return SYN_OK on matched segment processing, SYN_ERROR on invalid/unmatched segment.
 */
SYN_Status syn_tcp_process_packet(SYN_TCP *tcp, const uint8_t *ip_packet, size_t len,
                                  uint8_t *tx_out, size_t *tx_len);

/**
 * @brief Compute TCP 16-bit 1's complement checksum over pseudo-header and segment.
 * @param src_ip  Source 32-bit IPv4 address.
 * @param dst_ip  Destination 32-bit IPv4 address.
 * @param tcp_seg Pointer to raw TCP segment bytes.
 * @param len     TCP segment length in bytes.
 * @return Computed 16-bit TCP checksum.
 */
uint16_t syn_tcp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *tcp_seg, size_t len);

/* ── Task Blocking Macros ───────────────────────────────────────────────── */

/**
 * @brief Validate pointer arguments for PT_TCP_BLOCK_READ without triggering -Waddress.
 * @param pt         Protothread state pointer.
 * @param task       Task instance pointer.
 * @param conn       TCP connection pointer.
 * @param buf        Destination read buffer pointer.
 * @param bytes_read Output byte count pointer.
 * @return true if all pointers are non-NULL, false otherwise (reinitializing pt if non-NULL).
 */
static inline bool syn_tcp_pt_args_valid(SYN_PT *pt, const SYN_Task *task, const SYN_TcpConn *conn,
                                         const void *buf, const size_t *bytes_read)
{
    if (pt == NULL || task == NULL || conn == NULL || buf == NULL || bytes_read == NULL) {
        if (pt != NULL) {
            PT_INIT(pt);
        }
        return false;
    }
    return true;
}

/**
 * @brief Protothread helper macro to block task until TCP data is available.
 * @param pt         Protothread state.
 * @param task       Task instance.
 * @param conn       TCP connection pointer.
 * @param buf        Destination read buffer.
 * @param max_len    Maximum bytes to read.
 * @param bytes_read Output variable receiving count of read bytes.
 */
#define PT_TCP_BLOCK_READ(pt, task, conn, buf, max_len, bytes_read)                  \
    do {                                                                             \
        while (1) {                                                                  \
            if (!syn_tcp_pt_args_valid((pt), (task), (conn), (buf), (bytes_read))) { \
                return PT_EXITED;                                                    \
            }                                                                        \
            (conn)->blocked_task = (task);                                           \
            syn_port_enter_critical();                                               \
            if ((task)->state == (uint8_t)SYN_TASK_READY) {                          \
                (task)->state = (uint8_t)SYN_TASK_BLOCKED;                           \
                SYN_PT_STATE_BARRIER();                                              \
            }                                                                        \
            syn_port_exit_critical();                                                \
            if ((conn)->rx_len > 0U || (conn)->state != SYN_TCP_ESTABLISHED) {       \
                syn_port_enter_critical();                                           \
                if ((task)->state == (uint8_t)SYN_TASK_BLOCKED) {                    \
                    (task)->state = (uint8_t)SYN_TASK_READY;                         \
                }                                                                    \
                syn_port_exit_critical();                                            \
                break;                                                               \
            }                                                                        \
            PT_PASS(pt);                                                             \
        }                                                                            \
        (conn)->blocked_task = NULL;                                                 \
        if ((conn)->rx_len > 0U) {                                                   \
            size_t _n = ((conn)->rx_len < (max_len)) ? (conn)->rx_len : (max_len);   \
            if (_n > 0U) {                                                           \
                memcpy((buf), (conn)->rx_buf, _n);                                   \
                memmove((conn)->rx_buf, (conn)->rx_buf + _n, (conn)->rx_len - _n);   \
                (conn)->rx_len -= (uint16_t)_n;                                      \
            }                                                                        \
            *(bytes_read) = _n;                                                      \
        } else {                                                                     \
            *(bytes_read) = 0U;                                                      \
        }                                                                            \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_TCP_H */
