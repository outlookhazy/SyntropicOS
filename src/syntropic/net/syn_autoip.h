/**
 * @file syn_autoip.h
 * @brief Zero-Heap Native RFC 3927 AutoIP (Link-Local 169.254.x.x) Engine.
 *
 * Specifications:
 * - RFC 3927 Dynamic Configuration of IPv4 Link-Local Addresses
 * - Address Range: 169.254.1.0 to 169.254.254.255
 * - Zero dynamic memory allocation (0 bytes heap)
 */

#ifndef SYN_AUTOIP_H
#define SYN_AUTOIP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_AUTOIP_NETMASK 0xFFFF0000UL /**< Link-local network mask 255.255.0.0 */
#define SYN_AUTOIP_BASE_IP 0xA9FE0101UL /**< Link-local base IP address 169.254.1.1 */

/** AutoIP State Machine Enum. */
typedef enum {
    SYN_AUTOIP_STATE_INIT = 0, /**< Uninitialized state */
    SYN_AUTOIP_STATE_PROBE,    /**< Sending ARP probes state */
    SYN_AUTOIP_STATE_ANNOUNCE, /**< Sending ARP announcement state */
    SYN_AUTOIP_STATE_BOUND     /**< Successfully bound to IP state */
} SYN_AUTOIP_State;

/** AutoIP Context Descriptor. */
typedef struct {
    SYN_AUTOIP_State state; /**< Active state machine state */
    uint32_t ip_addr;       /**< Candidate Link-Local IP */
    uint8_t probe_count;    /**< Number of ARP probes sent */
    uint8_t announce_count; /**< Number of ARP announcements sent */
    uint32_t collisions;    /**< Collision counter */
} SYN_AUTOIP;

/**
 * @brief Initialize RFC 3927 AutoIP Engine Context.
 *
 * @param autoip   Pointer to AutoIP context.
 * @param mac_addr 6-byte client MAC address (used to seed candidate IP selection).
 * @return SYN_OK on success.
 */
SYN_Status syn_autoip_init(SYN_AUTOIP *autoip, const uint8_t mac_addr[6]);

/**
 * @brief Build an ARP Probe frame to verify IP availability.
 *
 * @param autoip   Pointer to AutoIP context.
 * @param mac_addr 6-byte client MAC address.
 * @param buf_out  Output frame buffer (must hold at least 60 bytes).
 * @param len_out  Pointer to receive length.
 * @return SYN_OK on success.
 */
SYN_Status syn_autoip_build_probe(SYN_AUTOIP *autoip, const uint8_t mac_addr[6], uint8_t *buf_out,
                                  size_t *len_out);

/**
 * @brief Build an ARP Announcement frame to announce claimed IP.
 *
 * @param autoip   Pointer to AutoIP context.
 * @param mac_addr 6-byte client MAC address.
 * @param buf_out  Output frame buffer (must hold at least 60 bytes).
 * @param len_out  Pointer to receive length.
 * @return SYN_OK on success.
 */
SYN_Status syn_autoip_build_announce(SYN_AUTOIP *autoip, const uint8_t mac_addr[6],
                                     uint8_t *buf_out, size_t *len_out);

/**
 * @brief Process ARP response frame during probing.
 *
 * @param autoip   Pointer to AutoIP context.
 * @param eth      Pointer to Ethernet context (auto-updated when BOUND).
 * @param arp_frame Pointer to received frame.
 * @param len      Length in bytes.
 * @return SYN_OK when BOUND, SYN_BUSY if conflict detected and new candidate picked.
 */
SYN_Status syn_autoip_process_arp(SYN_AUTOIP *autoip, SYN_ETH *eth, const uint8_t *arp_frame,
                                  size_t len);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until AutoIP link-local binding completes.
 *
 * @param pt     Protothread context.
 * @param autoip Pointer to AutoIP context.
 */
#define PT_AUTOIP_WAIT_BOUND(pt, autoip) \
    PT_WAIT_UNTIL(pt, (autoip)->state == SYN_AUTOIP_STATE_BOUND)

/**
 * @brief Block task execution (SYN_TASK_BLOCKED) until AutoIP link-local binding completes.
 * @note Driver/timer caller must invoke syn_task_resume(task) when AutoIP reaches BOUND state.
 *
 * @param pt     Protothread context.
 * @param task   Pointer to SYN_Task.
 * @param autoip Pointer to AutoIP context.
 */
#define PT_AUTOIP_BLOCK_BOUND(pt, task, autoip) \
    PT_BLOCK_CONDITION(pt, task, (autoip)->state == SYN_AUTOIP_STATE_BOUND)

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUTOIP_H */
