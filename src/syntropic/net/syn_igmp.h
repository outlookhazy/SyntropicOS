/**
 * @file syn_igmp.h
 * @brief Zero-Heap Native IGMPv2 Protocol Engine.
 *
 * Specifications:
 * - RFC 2236 Internet Group Management Protocol, Version 2
 * - IP Protocol Number: 2
 * - Zero dynamic memory allocation (0 bytes heap)
 * - Types:
 *   - 0x11: Membership Query
 *   - 0x16: IGMPv2 Membership Report (Join)
 *   - 0x17: IGMPv2 Leave Group
 */

#ifndef SYN_IGMP_H
#define SYN_IGMP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_IGMP_TYPE_MEMBERSHIP_QUERY 0x11U /**< IGMP Membership Query message type (0x11) */
#define SYN_IGMP_TYPE_V2_REPORT 0x16U        /**< IGMPv2 Membership Report message type (0x16) */
#define SYN_IGMP_TYPE_V2_LEAVE 0x17U         /**< IGMPv2 Leave Group message type (0x17) */

#define SYN_IGMP_MAX_GROUPS 4U /**< Maximum concurrent multicast groups tracked per engine */

/** Multicast Group Record. */
typedef struct {
    uint32_t group_ip; /**< Multicast group IPv4 address */
    bool joined;       /**< True if currently joined to group */
} SYN_IGMP_Group;

/** IGMP Engine Context Descriptor. */
typedef struct {
    SYN_IGMP_Group groups[SYN_IGMP_MAX_GROUPS]; /**< Multicast groups tracking table */
    uint32_t reports_sent;                      /**< Total IGMP reports sent */
    uint32_t queries_received;                  /**< Total IGMP queries received */
    uint32_t leaves_sent;                       /**< Total IGMP leave messages sent */
} SYN_IGMP;

/**
 * @brief Initialize IGMP Engine Context.
 *
 * @param igmp Pointer to IGMP context.
 * @return SYN_OK on success.
 */
SYN_Status syn_igmp_init(SYN_IGMP *igmp);

/**
 * @brief Build an IGMPv2 Membership Report (Join Group) or Leave frame over Ethernet.
 *
 * @param igmp       Pointer to IGMP context.
 * @param eth        Pointer to Ethernet context.
 * @param type       SYN_IGMP_TYPE_V2_REPORT or SYN_IGMP_TYPE_V2_LEAVE.
 * @param group_ip   32-bit IPv4 multicast address.
 * @param frame_out  Output frame buffer (must hold at least 60 bytes).
 * @param frame_len  Pointer to receive final Ethernet II frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_igmp_build_report(SYN_IGMP *igmp, SYN_ETH *eth, uint8_t type, uint32_t group_ip,
                                 uint8_t *frame_out, size_t *frame_len);

/**
 * @brief Join an IPv4 Multicast Group and construct IGMPv2 Join Frame.
 *
 * @param igmp      Pointer to IGMP context.
 * @param eth       Pointer to Ethernet context.
 * @param group_ip  32-bit IPv4 multicast address (e.g. 224.0.0.251 for mDNS).
 * @param frame_out Output frame buffer (must hold at least 60 bytes).
 * @param frame_len Pointer to receive final Ethernet II frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_igmp_join_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                               size_t *frame_len);

/**
 * @brief Leave an IPv4 Multicast Group and construct IGMPv2 Leave Frame.
 *
 * @param igmp      Pointer to IGMP context.
 * @param eth       Pointer to Ethernet context.
 * @param group_ip  32-bit IPv4 multicast address.
 * @param frame_out Output frame buffer (must hold at least 60 bytes).
 * @param frame_len Pointer to receive final Ethernet II frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_igmp_leave_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                                size_t *frame_len);

/**
 * @brief Process incoming IGMP packet (e.g. Membership Query from switch).
 *
 * @param igmp     Pointer to IGMP context.
 * @param eth      Pointer to Ethernet context.
 * @param ip_pkt   Pointer to incoming IPv4 packet (starting at Ethernet frame).
 * @param len      Length in bytes.
 * @param frame_tx Output frame buffer to receive reply report (if generated).
 * @param tx_len   Pointer to receive byte length of generated reply frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_igmp_process_packet(SYN_IGMP *igmp, SYN_ETH *eth, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IGMP_H */
