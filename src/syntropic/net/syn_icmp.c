/**
 * @file syn_icmp.c
 * @brief Zero-Heap Native ICMP Protocol Engine Implementation.
 */

#include "syntropic/net/syn_icmp.h"

#include <string.h>

SYN_Status syn_icmp_init(SYN_ICMP *icmp)
{
    if (!icmp) {
        return SYN_INVALID_PARAM;
    }
    memset(icmp, 0, sizeof(*icmp));
    return SYN_OK;
}

uint16_t syn_icmp_checksum(const void *buf, size_t len)
{
    return syn_ip_checksum(buf, len);
}

/**
 * @brief Verify Echo Request destination MAC and IPv4 address are not broadcast or multicast.
 *
 * @param ip_pkt Inbound Ethernet + IPv4 packet.
 * @return true if destination MAC and IPv4 may be reflected in Echo Reply, false otherwise.
 */
static bool icmp_echo_dst_unicast(const uint8_t *ip_pkt)
{
    if ((ip_pkt[0] & 0x01U) != 0U) {
        return false;
    }
    uint32_t dst_ip = ((uint32_t)ip_pkt[30] << 24U) | ((uint32_t)ip_pkt[31] << 16U) |
                      ((uint32_t)ip_pkt[32] << 8U) | (uint32_t)ip_pkt[33];
    return (dst_ip != 0xFFFFFFFFUL && (dst_ip & 0xF0000000UL) != 0xE0000000UL);
}

/**
 * @brief Build an Ethernet II + IPv4 + ICMP Echo Reply frame from a validated Echo Request.
 *
 * @param icmp        Pointer to ICMP context.
 * @param ip_pkt      Inbound Ethernet + IPv4 packet.
 * @param pkt_len     Unpadded Ethernet + IPv4 packet length (14 + ip_total_len).
 * @param icmp_offset Offset of ICMP header inside ip_pkt.
 * @param frame_tx    Destination buffer for reply frame.
 * @param tx_len      Output wire frame length (padded to 60B minimum).
 */
static void icmp_build_echo_reply(SYN_ICMP *icmp, const uint8_t *ip_pkt, size_t pkt_len,
                                  size_t icmp_offset, uint8_t *frame_tx, size_t *tx_len)
{
    if (frame_tx == NULL || tx_len == NULL) {
        return;
    }

    memcpy(frame_tx, ip_pkt, pkt_len);

    /* Swap MAC addresses (offset 0 and 6) */
    memcpy(&frame_tx[0], &ip_pkt[6], 6U);
    memcpy(&frame_tx[6], &ip_pkt[0], 6U);

    /* Swap IP addresses (offset 26 and 30) */
    memcpy(&frame_tx[26], &ip_pkt[30], 4U);
    memcpy(&frame_tx[30], &ip_pkt[26], 4U);

    /* Modify ICMP Type: 8 (Request) -> 0 (Reply) */
    frame_tx[icmp_offset] = SYN_ICMP_TYPE_ECHO_REPLY;
    frame_tx[icmp_offset + 2U] = 0U;
    frame_tx[icmp_offset + 3U] = 0U;

    size_t icmp_len = pkt_len - icmp_offset;
    uint16_t csum = syn_icmp_checksum(&frame_tx[icmp_offset], icmp_len);
    frame_tx[icmp_offset + 2U] = (uint8_t)(csum >> 8U);
    frame_tx[icmp_offset + 3U] = (uint8_t)(csum & 0xFFU);

    /* Zero pad frame to minimum Ethernet length (60 bytes) */
    size_t wire_len = pkt_len;
    if (wire_len < SYN_ETH_MIN_FRAME_LEN) {
        memset(&frame_tx[wire_len], 0, SYN_ETH_MIN_FRAME_LEN - wire_len);
        wire_len = SYN_ETH_MIN_FRAME_LEN;
    }

    icmp->echo_replies_tx++;
    *tx_len = wire_len;
}

/**
 * @brief Validate IPv4 total length and ICMP checksum, returning unpadded lengths.
 *
 * @param icmp         Pointer to ICMP context.
 * @param ip_pkt       Inbound Ethernet + IPv4 packet.
 * @param len          Raw frame byte length.
 * @param ip_hl        Validated IPv4 header byte length (>= 20).
 * @param pkt_len_out  Output unpadded frame length (14 + ip_total_len).
 * @param icmp_off_out Output ICMP header byte offset.
 * @return SYN_OK on valid ICMP packet, SYN_INVALID_PARAM or SYN_ERROR otherwise.
 */
static SYN_Status icmp_validate_bounds_and_csum(SYN_ICMP *icmp, const uint8_t *ip_pkt, size_t len,
                                                uint8_t ip_hl, size_t *pkt_len_out,
                                                size_t *icmp_off_out)
{
    uint16_t ip_total_len = ((uint16_t)ip_pkt[16] << 8U) | (uint16_t)ip_pkt[17];
    size_t pkt_len = 14U + (size_t)ip_total_len;
    if (ip_total_len < (uint16_t)ip_hl + SYN_ICMP_HEADER_LEN || pkt_len > len ||
        pkt_len > SYN_ETH_MAX_FRAME_LEN) {
        icmp->errors++;
        return SYN_INVALID_PARAM;
    }

    size_t icmp_offset = 14U + (size_t)ip_hl;
    size_t icmp_len = (size_t)ip_total_len - (size_t)ip_hl;
    if (syn_icmp_checksum(&ip_pkt[icmp_offset], icmp_len) != 0U) {
        icmp->errors++;
        return SYN_ERROR;
    }

    *pkt_len_out = pkt_len;
    *icmp_off_out = icmp_offset;
    return SYN_OK;
}

SYN_Status syn_icmp_process_packet(SYN_ICMP *icmp, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len)
{
    if (!icmp || !ip_pkt || len < 34U) {
        return SYN_INVALID_PARAM;
    }

    if (tx_len) {
        *tx_len = 0U;
    }

    /* Verify Ethernet EtherType == IPv4 (0x0800) */
    uint16_t ethertype = ((uint16_t)ip_pkt[12] << 8U) | (uint16_t)ip_pkt[13];
    if (ethertype != SYN_ETHTYPE_IPV4) {
        return SYN_OK;
    }

    /* IP Header begins at offset 14 */
    const uint8_t *ip_hdr = &ip_pkt[14];
    uint8_t ip_hl = (uint8_t)((ip_hdr[0] & 0x0FU) * 4U);
    if (ip_hl < 20U || (14U + (size_t)ip_hl) > len) {
        icmp->errors++;
        return SYN_INVALID_PARAM;
    }

    if (ip_hdr[9] != 1U) { /* Protocol 1 = ICMP */
        return SYN_OK;
    }

    size_t pkt_len = 0U;
    size_t icmp_offset = 0U;
    SYN_Status st = icmp_validate_bounds_and_csum(icmp, ip_pkt, len, ip_hl, &pkt_len, &icmp_offset);
    if (st != SYN_OK) {
        return st;
    }

    uint8_t type = ip_pkt[icmp_offset];
    if (type == SYN_ICMP_TYPE_ECHO_REQUEST) {
        if (!icmp_echo_dst_unicast(ip_pkt)) {
            return SYN_OK;
        }
        icmp->echo_requests_rx++;
        icmp_build_echo_reply(icmp, ip_pkt, pkt_len, icmp_offset, frame_tx, tx_len);
    } else if (type == SYN_ICMP_TYPE_ECHO_REPLY) {
        icmp->echo_replies_rx++;
    }

    return SYN_OK;
}

SYN_Status syn_icmp_build_echo_request(SYN_ICMP *icmp, SYN_ETH *eth, uint32_t dst_ip,
                                       const uint8_t dst_mac[6], uint16_t id, uint16_t seq,
                                       const uint8_t *payload, size_t payload_len,
                                       uint8_t *frame_out, size_t *frame_len)
{
    if (!icmp || !eth || !dst_mac || !frame_out || !frame_len) {
        return SYN_INVALID_PARAM;
    }

    if (payload_len > (65535U - 28U) || (!payload && payload_len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    uint16_t icmp_len = (uint16_t)(SYN_ICMP_HEADER_LEN + payload_len);
    size_t total_len = 14U + 20U + (size_t)icmp_len;
    if (total_len < SYN_ETH_MIN_FRAME_LEN) {
        total_len = SYN_ETH_MIN_FRAME_LEN;
    }
    if (total_len > SYN_ETH_MAX_FRAME_LEN) {
        return SYN_INVALID_PARAM;
    }

    memset(frame_out, 0, total_len);

    /* Ethernet II Header */
    /* LCOV_EXCL_START: Unreachable parameter bounds check */
    if (syn_eth_pack_header(frame_out, total_len, dst_mac, eth->mac_addr, SYN_ETHTYPE_IPV4) == 0) {
        return SYN_INVALID_PARAM;
    }

    /* IPv4 Header */
    size_t ip_bytes =
        syn_ip_pack_header(&frame_out[14], total_len - 14, eth->ip_addr, dst_ip, 1, icmp_len, id);
    if (ip_bytes == 0) {
        return SYN_INVALID_PARAM;
    }
    /* LCOV_EXCL_STOP */

    /* ICMP Header */
    size_t icmp_off = 34;
    frame_out[icmp_off] = SYN_ICMP_TYPE_ECHO_REQUEST;
    frame_out[icmp_off + 1] = 0; /* Code */
    frame_out[icmp_off + 2] = 0;
    frame_out[icmp_off + 3] = 0; /* Checksum placeholder */
    frame_out[icmp_off + 4] = (uint8_t)(id >> 8);
    frame_out[icmp_off + 5] = (uint8_t)(id & 0xFF);
    frame_out[icmp_off + 6] = (uint8_t)(seq >> 8);
    frame_out[icmp_off + 7] = (uint8_t)(seq & 0xFF);

    if (payload && payload_len > 0) {
        memcpy(&frame_out[icmp_off + 8], payload, payload_len);
    }

    uint16_t icmp_csum = syn_icmp_checksum(&frame_out[icmp_off], icmp_len);
    frame_out[icmp_off + 2] = (uint8_t)(icmp_csum >> 8);
    frame_out[icmp_off + 3] = (uint8_t)(icmp_csum & 0xFF);

    icmp->echo_requests_tx++;
    *frame_len = total_len;

    return SYN_OK;
}
