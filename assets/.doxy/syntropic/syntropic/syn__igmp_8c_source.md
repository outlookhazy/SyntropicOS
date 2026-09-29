

# File syn\_igmp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_igmp.c**](syn__igmp_8c.md)

[Go to the documentation of this file](syn__igmp_8c.md)


```C++

#include "syntropic/net/syn_igmp.h"

#include "syntropic/net/syn_icmp.h" /* For syn_icmp_checksum */

#include <string.h>

SYN_Status syn_igmp_init(SYN_IGMP *igmp)
{
    if (!igmp) {
        return SYN_INVALID_PARAM;
    }
    memset(igmp, 0, sizeof(*igmp));
    return SYN_OK;
}

SYN_Status syn_igmp_build_report(SYN_IGMP *igmp, SYN_ETH *eth, uint8_t type, uint32_t group_ip,
                                 uint8_t *frame_out, size_t *frame_len)
{
    if (!igmp || !eth || !frame_out || !frame_len || group_ip == 0) {
        return SYN_INVALID_PARAM;
    }

    size_t ip_len = 20 + 8; /* IP header (20) + IGMP payload (8) */
    size_t total_len = 14 + ip_len;
    if (total_len < SYN_ETH_MIN_FRAME_LEN) {
        total_len = SYN_ETH_MIN_FRAME_LEN;
    }

    memset(frame_out, 0, total_len);

    /* Multicast Destination MAC: 01:00:5E: (Group IP & 0x7FFFFF) */
    uint32_t group_low = group_ip & 0x007FFFFFUL;
    uint8_t mcast_mac[6];
    mcast_mac[0] = 0x01;
    mcast_mac[1] = 0x00;
    mcast_mac[2] = 0x5E;
    mcast_mac[3] = (uint8_t)(group_low >> 16);
    mcast_mac[4] = (uint8_t)(group_low >> 8);
    mcast_mac[5] = (uint8_t)(group_low);

    /* Ethernet II Header */
    /* LCOV_EXCL_START: Unreachable parameter bounds check */
    if (syn_eth_pack_header(frame_out, total_len, mcast_mac, eth->mac_addr, SYN_ETHTYPE_IPV4) ==
        0) {
        return SYN_INVALID_PARAM;
    }

    /* IPv4 Header (Protocol 2 = IGMP, TTL = 1) */
    size_t ip_bytes =
        syn_ip_pack_header(&frame_out[14], total_len - 14, eth->ip_addr, group_ip, 2, 8, 0x1234);
    if (ip_bytes == 0) {
        return SYN_INVALID_PARAM;
    }
    /* LCOV_EXCL_STOP */
    frame_out[22] = 1; /* Override TTL = 1 for IGMP multicast */

    /* Re-calculate IP checksum due to TTL override */
    uint16_t ip_csum = syn_ip_checksum(&frame_out[14], 20);
    frame_out[24] = (uint8_t)(ip_csum >> 8);
    frame_out[25] = (uint8_t)(ip_csum & 0xFF);

    /* IGMP Header */
    size_t igmp_off = 34;
    frame_out[igmp_off] = type;
    frame_out[igmp_off + 1] = 0; /* Max Resp Time */
    frame_out[igmp_off + 2] = 0;
    frame_out[igmp_off + 3] = 0;

    frame_out[igmp_off + 4] = (uint8_t)(group_ip >> 24);
    frame_out[igmp_off + 5] = (uint8_t)(group_ip >> 16);
    frame_out[igmp_off + 6] = (uint8_t)(group_ip >> 8);
    frame_out[igmp_off + 7] = (uint8_t)(group_ip);

    uint16_t igmp_csum = syn_ip_checksum(&frame_out[igmp_off], 8);
    frame_out[igmp_off + 2] = (uint8_t)(igmp_csum >> 8);
    frame_out[igmp_off + 3] = (uint8_t)(igmp_csum & 0xFF);

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
    if (!igmp) {
        return SYN_INVALID_PARAM;
    }

    /* Record group membership */
    bool found = false;
    for (size_t i = 0; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (igmp->groups[i].group_ip == group_ip || !igmp->groups[i].joined) {
            igmp->groups[i].group_ip = group_ip;
            igmp->groups[i].joined = true;
            found = true;
            break;
        }
    }
    if (!found) {
        igmp->groups[0].group_ip = group_ip;
        igmp->groups[0].joined = true;
    }

    return syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_REPORT, group_ip, frame_out,
                                 frame_len);
}

SYN_Status syn_igmp_leave_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                                size_t *frame_len)
{
    if (!igmp) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < SYN_IGMP_MAX_GROUPS; i++) {
        if (igmp->groups[i].group_ip == group_ip) {
            igmp->groups[i].joined = false;
            break;
        }
    }

    return syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_LEAVE, group_ip, frame_out, frame_len);
}

SYN_Status syn_igmp_process_packet(SYN_IGMP *igmp, SYN_ETH *eth, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len)
{
    if (!igmp || !ip_pkt || len < 42) {
        return SYN_INVALID_PARAM;
    }

    if (tx_len) {
        *tx_len = 0;
    }

    /* Verify EtherType == IPv4 */
    uint16_t ethertype = ((uint16_t)ip_pkt[12] << 8) | ip_pkt[13];
    if (ethertype != SYN_ETHTYPE_IPV4) {
        return SYN_OK;
    }

    /* Verify Protocol == 2 (IGMP) */
    uint8_t proto = ip_pkt[23];
    if (proto != 2) {
        return SYN_OK;
    }

    uint8_t type = ip_pkt[34];
    if (type == SYN_IGMP_TYPE_MEMBERSHIP_QUERY) {
        igmp->queries_received++;

        /* Respond with report for first joined group */
        for (size_t i = 0; i < SYN_IGMP_MAX_GROUPS; i++) {
            if (igmp->groups[i].joined && frame_tx && tx_len) {
                return syn_igmp_build_report(igmp, eth, SYN_IGMP_TYPE_V2_REPORT,
                                             igmp->groups[i].group_ip, frame_tx, tx_len);
            }
        }
    }

    return SYN_OK;
}
```


