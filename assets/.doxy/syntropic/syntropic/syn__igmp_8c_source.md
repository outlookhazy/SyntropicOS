

# File syn\_igmp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_igmp.c**](syn__igmp_8c.md)

[Go to the documentation of this file](syn__igmp_8c.md)


```C++


#include "syntropic/net/syn_igmp.h"

#include "syntropic/net/syn_icmp.h" /* For syn_icmp_checksum */

#include <string.h>

static void igmp_record_join(SYN_IGMP *igmp, uint32_t group_ip)
{
    for (size_t i = 0U; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (igmp->groups[i].joined && igmp->groups[i].group_ip == group_ip) {
            return;
        }
    }
    for (size_t i = 0U; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (!igmp->groups[i].joined) {
            igmp->groups[i].group_ip = group_ip;
            igmp->groups[i].joined = true;
            return;
        }
    }
    igmp->groups[0].group_ip = group_ip;
    igmp->groups[0].joined = true;
}

static uint32_t igmp_select_query_group(const SYN_IGMP *igmp, uint32_t query_group)
{
    for (size_t i = 0U; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (igmp->groups[i].joined &&
            (query_group == 0U || igmp->groups[i].group_ip == query_group)) {
            return igmp->groups[i].group_ip;
        }
    }
    return 0U;
}

SYN_Status syn_igmp_init(SYN_IGMP *igmp)
{
    if (igmp == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(igmp, 0, sizeof(*igmp));
    return SYN_OK;
}

SYN_Status syn_igmp_build_report(SYN_IGMP *igmp, SYN_ETH *eth, uint8_t type, uint32_t group_ip,
                                 uint8_t *frame_out, size_t *frame_len)
{
    if (igmp == NULL || eth == NULL || frame_out == NULL || frame_len == NULL || group_ip == 0U) {
        return SYN_INVALID_PARAM;
    }

    size_t ip_len = 20U + 8U; /* IP header (20) + IGMP payload (8) */
    size_t total_len = 14U + ip_len;
    if (total_len < SYN_ETH_MIN_FRAME_LEN) {
        total_len = SYN_ETH_MIN_FRAME_LEN;
    }

    memset(frame_out, 0, total_len);

    /* RFC 2236 Section 3: Leave is sent to all-routers (224.0.0.2); Report to group_ip */
    uint32_t dst_ip = (type == SYN_IGMP_TYPE_V2_LEAVE) ? 0xE0000002UL : group_ip;
    uint32_t dst_low = dst_ip & 0x007FFFFFUL;
    uint8_t mcast_mac[6];
    mcast_mac[0] = 0x01U;
    mcast_mac[1] = 0x00U;
    mcast_mac[2] = 0x5EU;
    mcast_mac[3] = (uint8_t)(dst_low >> 16U);
    mcast_mac[4] = (uint8_t)(dst_low >> 8U);
    mcast_mac[5] = (uint8_t)(dst_low);

    /* Ethernet II Header */
    /* LCOV_EXCL_START: Unreachable parameter bounds check */
    if (syn_eth_pack_header(frame_out, total_len, mcast_mac, eth->mac_addr, SYN_ETHTYPE_IPV4) ==
        0U) {
        return SYN_INVALID_PARAM;
    }

    /* IPv4 Header (Protocol 2 = IGMP, TTL = 1) */
    size_t ip_bytes =
        syn_ip_pack_header(&frame_out[14], total_len - 14U, eth->ip_addr, dst_ip, 2U, 8U, 0x1234U);
    if (ip_bytes == 0U) {
        return SYN_INVALID_PARAM;
    }
    /* LCOV_EXCL_STOP */
    frame_out[22] = 1U; /* Override TTL = 1 for IGMP multicast */
    frame_out[24] = 0U;
    frame_out[25] = 0U;

    /* Re-calculate IP checksum due to TTL override */
    uint16_t ip_csum = syn_ip_checksum(&frame_out[14], 20U);
    frame_out[24] = (uint8_t)(ip_csum >> 8U);
    frame_out[25] = (uint8_t)(ip_csum & 0xFFU);

    /* IGMP Header */
    size_t igmp_off = 34U;
    frame_out[igmp_off] = type;
    frame_out[igmp_off + 1U] = 0U; /* Max Resp Time */
    frame_out[igmp_off + 2U] = 0U;
    frame_out[igmp_off + 3U] = 0U;

    frame_out[igmp_off + 4U] = (uint8_t)(group_ip >> 24U);
    frame_out[igmp_off + 5U] = (uint8_t)(group_ip >> 16U);
    frame_out[igmp_off + 6U] = (uint8_t)(group_ip >> 8U);
    frame_out[igmp_off + 7U] = (uint8_t)(group_ip);

    uint16_t igmp_csum = syn_ip_checksum(&frame_out[igmp_off], 8U);
    frame_out[igmp_off + 2U] = (uint8_t)(igmp_csum >> 8U);
    frame_out[igmp_off + 3U] = (uint8_t)(igmp_csum & 0xFFU);

    if (type == SYN_IGMP_TYPE_V2_REPORT) {
        igmp->reports_sent++;
    } else if (type == SYN_IGMP_TYPE_V2_LEAVE) {
        igmp->leaves_sent++;
    }

    *frame_len = total_len;
    return SYN_OK;
}

SYN_Status syn_igmp_join_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                               size_t *frame_len)
{
    if (igmp == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_Status status =
        syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_REPORT, group_ip, frame_out, frame_len);
    if (status == SYN_OK) {
        igmp_record_join(igmp, group_ip);
    }
    return status;
}

SYN_Status syn_igmp_leave_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                                size_t *frame_len)
{
    if (igmp == NULL) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0U; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (igmp->groups[i].joined && igmp->groups[i].group_ip == group_ip) {
            igmp->groups[i].joined = false;
            break;
        }
    }

    return syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_LEAVE, group_ip, frame_out, frame_len);
}

SYN_Status syn_igmp_process_packet(SYN_IGMP *igmp, SYN_ETH *eth, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len)
{
    if (igmp == NULL || ip_pkt == NULL || len < 42U) {
        return SYN_INVALID_PARAM;
    }

    if (tx_len != NULL) {
        *tx_len = 0U;
    }

    /* Verify EtherType == IPv4 */
    uint16_t ethertype = ((uint16_t)ip_pkt[12] << 8U) | ip_pkt[13];
    if (ethertype != SYN_ETHTYPE_IPV4) {
        return SYN_OK;
    }

    size_t ihl = (size_t)(ip_pkt[14] & 0x0FU) * 4U;
    if (ihl < 20U || (14U + ihl + 8U) > len) {
        return SYN_INVALID_PARAM;
    }

    /* Verify Protocol == 2 (IGMP) */
    uint8_t proto = ip_pkt[23];
    if (proto != 2U) {
        return SYN_OK;
    }

    size_t igmp_off = 14U + ihl;
    uint8_t type = ip_pkt[igmp_off];
    if (type == SYN_IGMP_TYPE_MEMBERSHIP_QUERY) {
        igmp->queries_received++;
        uint32_t query_group =
            ((uint32_t)ip_pkt[igmp_off + 4U] << 24U) | ((uint32_t)ip_pkt[igmp_off + 5U] << 16U) |
            ((uint32_t)ip_pkt[igmp_off + 6U] << 8U) | (uint32_t)ip_pkt[igmp_off + 7U];
        uint32_t report_group = igmp_select_query_group(igmp, query_group);
        if (report_group != 0U && frame_tx != NULL && tx_len != NULL) {
            return syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_REPORT, report_group, frame_tx,
                                         tx_len);
        }
    }

    return SYN_OK;
}
```


