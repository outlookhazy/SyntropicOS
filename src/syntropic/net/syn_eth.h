/**
 * @file syn_eth.h
 * @brief Zero-Heap Native Ethernet II & ARP Protocol Engine.
 *
 * Specifications:
 * - Ethernet II Header: [Dst MAC 6B] + [Src MAC 6B] + [EtherType 2B]
 * - Minimum Frame Size: 60 bytes (padded with zeros if payload < 46B)
 * - Maximum Frame Size: 1514 bytes (excluding 4B FCS)
 * - EtherTypes:
 *   - 0x0806: ARP (Address Resolution Protocol)
 *   - 0x0800: IPv4 (Internet Protocol v4)
 *   - 0x86DD: IPv6 (Internet Protocol v6)
 *   - 0x88A4: EtherCAT Industrial Ethernet
 */

#ifndef SYN_ETH_H
#define SYN_ETH_H

#include "port/syn_port_eth.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_ETH_MAC_LEN 6U          /**< Ethernet MAC address length in bytes */
#define SYN_ETH_HEADER_LEN 14U      /**< Ethernet II header length in bytes */
#define SYN_ETH_MIN_FRAME_LEN 60U   /**< Minimum Ethernet frame length with padding */
#define SYN_ETH_MAX_FRAME_LEN 1514U /**< Maximum standard Ethernet frame length */
#ifndef SYN_ETH_ARP_CACHE_SIZE
#define SYN_ETH_ARP_CACHE_SIZE 8U /**< Default ARP cache table capacity */
#endif

#define SYN_ETHTYPE_IPV4 0x0800U     /**< EtherType for IPv4 packets (0x0800) */
#define SYN_ETHTYPE_ARP 0x0806U      /**< EtherType for ARP packets (0x0806) */
#define SYN_ETHTYPE_IPV6 0x86DDU     /**< EtherType for IPv6 packets (0x86DD) */
#define SYN_ETHTYPE_ETHERCAT 0x88A4U /**< EtherType for EtherCAT frames (0x88A4) */

#define SYN_ARP_OP_REQUEST 1U /**< ARP Operation: Request (1) */
#define SYN_ARP_OP_REPLY 2U   /**< ARP Operation: Reply (2) */

/** Parsed Ethernet II Header. */
typedef struct {
    uint8_t dst_mac[SYN_ETH_MAC_LEN]; /**< Destination MAC address */
    uint8_t src_mac[SYN_ETH_MAC_LEN]; /**< Source MAC address */
    uint16_t ethertype;               /**< 16-bit EtherType field */
} SYN_ETH_Header;

/** ARP Table Cache Entry. */
typedef struct {
    uint32_t ip;                  /**< 32-bit IPv4 address */
    uint8_t mac[SYN_ETH_MAC_LEN]; /**< Resolved 6-byte MAC address */
    uint32_t last_seen_ms;        /**< Tick timestamp of last update */
    bool valid;                   /**< True if entry is active/valid */
} SYN_ETH_ArpEntry;

/** Native Ethernet Interface Engine. */
typedef struct {
    uint8_t mac_addr[SYN_ETH_MAC_LEN];                  /**< Interface MAC address */
    uint32_t ip_addr;                                   /**< Interface IPv4 address */
    uint32_t netmask;                                   /**< Interface subnet mask */
    uint32_t gateway;                                   /**< Default gateway IP address */
    SYN_ETH_ArpEntry arp_cache[SYN_ETH_ARP_CACHE_SIZE]; /**< ARP cache table array */
    uint32_t frames_tx;                                 /**< Total frames transmitted */
    uint32_t frames_rx;                                 /**< Total frames received */
    uint32_t arp_requests;                              /**< Total ARP requests processed */
    uint32_t arp_replies;                               /**< Total ARP replies processed */
} SYN_ETH;

/**
 * @brief Generate a deterministic Locally Administered MAC address from MCU Unique ID.
 *
 * @param uid_bytes Pointer to hardware unique ID bytes (e.g. STM32 96-bit UID).
 * @param uid_len   Length of unique ID in bytes.
 * @param mac_out   Output buffer to receive generated 6-byte MAC address.
 * @return SYN_OK on success.
 */
SYN_Status syn_eth_generate_mac(const void *uid_bytes, size_t uid_len, uint8_t mac_out[6]);

/**
 * @brief Initialize Native Ethernet Engine.
 *
 * @param eth     Pointer to Ethernet instance.
 * @param mac_addr 6-byte hardware MAC address.
 * @param ip_addr  32-bit IPv4 address (host byte order).
 * @return SYN_OK on success.
 */
SYN_Status syn_eth_init(SYN_ETH *eth, const uint8_t mac_addr[6], uint32_t ip_addr);

/**
 * @brief Process incoming Ethernet II frame.
 *
 * @param eth      Pointer to Ethernet instance.
 * @param frame    Pointer to raw frame bytes.
 * @param len      Frame byte length (60..1514 bytes).
 * @param tx_buf   Output buffer to receive immediate ARP reply if generated.
 * @param tx_len   Pointer to receive ARP reply byte length (0 if no reply needed).
 * @return SYN_OK on success.
 */
SYN_Status syn_eth_process_frame(SYN_ETH *eth, const uint8_t *frame, size_t len, uint8_t *tx_buf,
                                 size_t *tx_len);

/**
 * @brief Build and transmit an outbound Ethernet II frame.
 *
 * @param eth       Pointer to Ethernet instance.
 * @param dst_mac   Destination MAC address (6 bytes).
 * @param ethertype EtherType value (e.g. SYN_ETHTYPE_IPV4).
 * @param payload   Pointer to payload buffer.
 * @param payload_len Payload byte length.
 * @param frame_out Output frame buffer (must hold at least 1514 bytes).
 * @param frame_len Pointer to receive final raw Ethernet frame length.
 * @return SYN_OK on success.
 */
SYN_Status syn_eth_build_frame(SYN_ETH *eth, const uint8_t dst_mac[6], uint16_t ethertype,
                               const uint8_t *payload, size_t payload_len, uint8_t *frame_out,
                               size_t *frame_len);

/**
 * @brief Lookup MAC address in local ARP cache table.
 *
 * @param eth     Pointer to Ethernet instance.
 * @param ip      32-bit IPv4 address to lookup.
 * @param mac_out Output buffer (6 bytes) to receive resolved MAC.
 * @return SYN_OK if IP found in cache, SYN_NOT_FOUND if absent.
 */
SYN_Status syn_eth_arp_lookup(SYN_ETH *eth, uint32_t ip, uint8_t mac_out[6]);

/**
 * @brief Update or insert an entry into the local ARP cache table.
 *
 * @param eth Pointer to Ethernet instance.
 * @param ip  32-bit IPv4 address.
 * @param mac Resolved 6-byte MAC address.
 * @return SYN_OK on success.
 */
SYN_Status syn_eth_arp_update(SYN_ETH *eth, uint32_t ip, const uint8_t mac[6]);

/* ── Network Frame Serialization Helpers ───────────────────────────────── */

/**
 * @brief Calculate 16-bit Internet Checksum (RFC 1071) over buffer.
 *
 * @param buf Pointer to buffer.
 * @param len Buffer byte length.
 * @return 16-bit one's complement checksum.
 */
uint16_t syn_ip_checksum(const void *buf, size_t len);

/**
 * @brief Pack Ethernet II Header (14 bytes) into target buffer.
 *
 * @param buf       Output destination buffer.
 * @param buf_size  Destination buffer capacity.
 * @param dst_mac   Destination 6-byte MAC address.
 * @param src_mac   Source 6-byte MAC address.
 * @param ethertype 16-bit EtherType (host byte order).
 * @return Bytes written (14) on success, 0 on invalid param / insufficient buffer.
 */
size_t syn_eth_pack_header(uint8_t *buf, size_t buf_size, const uint8_t dst_mac[6],
                           const uint8_t src_mac[6], uint16_t ethertype);

/**
 * @brief Pack standard 20-byte IPv4 Header into target buffer.
 *
 * @param buf         Output destination buffer.
 * @param buf_size    Destination buffer capacity.
 * @param src_ip      Source IPv4 address (host byte order).
 * @param dst_ip      Destination IPv4 address (host byte order).
 * @param proto       IP Protocol number (1=ICMP, 2=IGMP, 6=TCP, 17=UDP).
 * @param payload_len Payload byte length after IPv4 header.
 * @param id          16-bit IP identification field.
 * @return Bytes written (20) on success, 0 on invalid param / insufficient buffer.
 */
size_t syn_ip_pack_header(uint8_t *buf, size_t buf_size, uint32_t src_ip, uint32_t dst_ip,
                          uint8_t proto, uint16_t payload_len, uint16_t id);

/* ── Protocol engine injection ──────────────────────────────────────────── */

/**
 * @brief Weak hook — override to inject an ICMP engine into the Ethernet dispatcher.
 *
 * When syn_eth_process_frame receives an IPv4/ICMP packet (protocol 1),
 * it calls this to obtain the ICMP instance. Return NULL to silently drop.
 *
 * @return Pointer to SYN_ICMP instance, or NULL.
 */
struct SYN_ICMP *syn_eth_get_icmp_instance(void);

/**
 * @brief Weak hook — override to inject a TCP engine into the Ethernet dispatcher.
 *
 * When syn_eth_process_frame receives an IPv4/TCP packet (protocol 6),
 * it calls this to obtain the TCP instance. Return NULL to silently drop.
 *
 * @return Pointer to SYN_TCP instance, or NULL.
 */
struct SYN_TCP *syn_eth_get_tcp_instance(void);

/**
 * @brief Set the IGMP engine instance for Ethernet IPv4 protocol 2 dispatch.
 *
 * @param igmp Pointer to SYN_IGMP instance, or NULL to detach.
 */
void syn_eth_set_igmp_instance(void *igmp);

/**
 * @brief Weak hook — obtain the IGMP engine instance for Ethernet IPv4 protocol 2 dispatch.
 *
 * @return Pointer to SYN_IGMP instance, or NULL.
 */
void *syn_eth_get_igmp_instance(void);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

/**
 * @brief Block a protothread coroutine until a raw Ethernet frame is received from PHY.
 *
 * @param pt      Protothread context.
 * @param rx_buf  Buffer to receive frame.
 * @param max_len Capacity.
 * @param rx_len  Pointer to receive length.
 */
#define PT_ETH_WAIT_FRAME(pt, rx_buf, max_len, rx_len) \
    PT_WAIT_UNTIL(pt, syn_port_eth_rx((rx_buf), (max_len), (rx_len)) == SYN_OK)

/**
 * @brief Block a protothread coroutine until an IP is resolved in the local ARP cache.
 *
 * @param pt      Protothread context.
 * @param eth     Pointer to Ethernet instance.
 * @param target_ip Target IPv4 address.
 * @param mac_out Output buffer (6 bytes).
 */
#define PT_ETH_WAIT_ARP(pt, eth, target_ip, mac_out) \
    PT_WAIT_UNTIL(pt, syn_eth_arp_lookup((eth), (target_ip), (mac_out)) == SYN_OK)

#ifdef __cplusplus
}
#endif

#endif /* SYN_ETH_H */
