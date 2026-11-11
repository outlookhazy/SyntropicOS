/**
 * @file syn_icmp.h
 * @brief Zero-Heap Native ICMP Protocol Engine.
 *
 * Specifications:
 * - RFC 792 Internet Control Message Protocol
 * - Zero dynamic memory allocation (0 bytes heap)
 * - Types:
 *   - Type 0: Echo Reply
 *   - Type 3: Destination Unreachable
 *   - Type 8: Echo Request
 */

#ifndef SYN_ICMP_H
#define SYN_ICMP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_ICMP_TYPE_ECHO_REPLY 0U   /**< ICMP Type 0: Echo Reply */
#define SYN_ICMP_TYPE_UNREACHABLE 3U  /**< ICMP Type 3: Destination Unreachable */
#define SYN_ICMP_TYPE_ECHO_REQUEST 8U /**< ICMP Type 8: Echo Request */

#define SYN_ICMP_HEADER_LEN 8U /**< Standard ICMP header length in bytes (8) */

/** ICMP Engine Context Descriptor. */
typedef struct SYN_ICMP {
    uint32_t echo_requests_rx; /**< Echo requests received counter */
    uint32_t echo_replies_tx;  /**< Echo replies transmitted counter */
    uint32_t echo_requests_tx; /**< Outbound echo requests transmitted counter */
    uint32_t echo_replies_rx;  /**< Outbound echo replies received counter */
    uint32_t errors;           /**< Checksum/format error counter */
} SYN_ICMP;

/**
 * @brief Initialize ICMP Engine Context.
 *
 * @param icmp Pointer to ICMP context.
 * @return SYN_OK on success.
 */
SYN_Status syn_icmp_init(SYN_ICMP *icmp);

/**
 * @brief Compute RFC 1071 Ones-Complement Internet Checksum.
 *
 * @param buf Pointer to buffer bytes.
 * @param len Length in bytes.
 * @return 16-bit Ones-Complement checksum.
 */
uint16_t syn_icmp_checksum(const void *buf, size_t len);

/**
 * @brief Process incoming ICMP packet inside IPv4 payload.
 *
 * Automatically generates a matching Type 0 Echo Reply if a Type 8 Echo Request is received.
 *
 * @param icmp     Pointer to ICMP context.
 * @param ip_pkt   Pointer to incoming Ethernet II + IPv4 frame.
 * @param len      Ethernet frame byte length.
 * @param frame_tx Output buffer to receive constructed Ethernet II reply frame
 *                 (must hold at least SYN_ETH_MAX_FRAME_LEN = 1514 bytes when non-NULL).
 * @param tx_len   Pointer to receive byte length of constructed reply frame (0 if none generated).
 * @return SYN_OK on success.
 */
SYN_Status syn_icmp_process_packet(SYN_ICMP *icmp, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len);

/**
 * @brief Construct an outbound ICMP Echo Request frame.
 *
 * @param icmp       Pointer to ICMP context.
 * @param eth        Pointer to Ethernet context.
 * @param dst_ip     32-bit IPv4 address of destination.
 * @param dst_mac    6-byte MAC address of destination.
 * @param id         Identifier.
 * @param seq        Sequence number.
 * @param payload    Pointer to payload data.
 * @param payload_len Payload length in bytes.
 * @param frame_out  Output frame buffer (must hold at least max(SYN_ETH_MIN_FRAME_LEN,
 *                   42 + payload_len) bytes, up to SYN_ETH_MAX_FRAME_LEN = 1514 bytes).
 * @param frame_len  Pointer to receive final Ethernet II frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_icmp_build_echo_request(SYN_ICMP *icmp, SYN_ETH *eth, uint32_t dst_ip,
                                       const uint8_t dst_mac[6], uint16_t id, uint16_t seq,
                                       const uint8_t *payload, size_t payload_len,
                                       uint8_t *frame_out, size_t *frame_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ICMP_H */
