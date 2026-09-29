

# File syn\_autoip.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_autoip.c**](syn__autoip_8c.md)

[Go to the documentation of this file](syn__autoip_8c.md)


```C++

#include "syntropic/net/syn_autoip.h"

#include <string.h>

SYN_Status syn_autoip_init(SYN_AUTOIP *autoip, const uint8_t mac_addr[6])
{
    if (!autoip || !mac_addr) {
        return SYN_INVALID_PARAM;
    }
    memset(autoip, 0, sizeof(*autoip));
    autoip->state = SYN_AUTOIP_STATE_INIT;

    /* Generate candidate IP in 169.254.1.1 .. 169.254.254.255 seeded by MAC address */
    uint16_t seed = ((uint16_t)mac_addr[4] << 8) | mac_addr[5];
    uint8_t octet3 = (uint8_t)(1 + (seed % 254));
    uint8_t octet4 = (uint8_t)(1 + ((seed >> 4) % 254));

    autoip->ip_addr = 0xA9FE0000UL | ((uint32_t)octet3 << 8) | octet4;
    return SYN_OK;
}

SYN_Status syn_autoip_build_probe(SYN_AUTOIP *autoip, const uint8_t mac_addr[6], uint8_t *buf_out,
                                  size_t *len_out)
{
    if (!autoip || !mac_addr || !buf_out || !len_out) {
        return SYN_INVALID_PARAM;
    }

    memset(buf_out, 0, 60);
    memset(&buf_out[0], 0xFF, 6); /* Broadcast Dst */
    memcpy(&buf_out[6], mac_addr, 6);
    buf_out[12] = 0x08;
    buf_out[13] = 0x06; /* EtherType = ARP */

    buf_out[14] = 0;
    buf_out[15] = 1; /* Ethernet */
    buf_out[16] = 0x08;
    buf_out[17] = 0x00; /* IPv4 */
    buf_out[18] = 6;
    buf_out[19] = 4;
    buf_out[20] = 0;
    buf_out[21] = 1; /* Request */

    memcpy(&buf_out[22], mac_addr, 6);
    /* Sender IP = 0.0.0.0 for ARP Probe */
    buf_out[28] = 0;
    buf_out[29] = 0;
    buf_out[30] = 0;
    buf_out[31] = 0;

    memset(&buf_out[32], 0, 6);
    buf_out[38] = (uint8_t)(autoip->ip_addr >> 24);
    buf_out[39] = (uint8_t)(autoip->ip_addr >> 16);
    buf_out[40] = (uint8_t)(autoip->ip_addr >> 8);
    buf_out[41] = (uint8_t)(autoip->ip_addr);

    autoip->state = SYN_AUTOIP_STATE_PROBE;
    autoip->probe_count++;
    *len_out = 60;

    return SYN_OK;
}

SYN_Status syn_autoip_build_announce(SYN_AUTOIP *autoip, const uint8_t mac_addr[6],
                                     uint8_t *buf_out, size_t *len_out)
{
    if (!autoip || !mac_addr || !buf_out || !len_out) {
        return SYN_INVALID_PARAM;
    }

    memset(buf_out, 0, 60);
    memset(&buf_out[0], 0xFF, 6);
    memcpy(&buf_out[6], mac_addr, 6);
    buf_out[12] = 0x08;
    buf_out[13] = 0x06;

    buf_out[14] = 0;
    buf_out[15] = 1;
    buf_out[16] = 0x08;
    buf_out[17] = 0x00;
    buf_out[18] = 6;
    buf_out[19] = 4;
    buf_out[20] = 0;
    buf_out[21] = 1;

    memcpy(&buf_out[22], mac_addr, 6);
    /* Sender IP = candidate IP for ARP Announcement */
    buf_out[28] = (uint8_t)(autoip->ip_addr >> 24);
    buf_out[29] = (uint8_t)(autoip->ip_addr >> 16);
    buf_out[30] = (uint8_t)(autoip->ip_addr >> 8);
    buf_out[31] = (uint8_t)(autoip->ip_addr);

    memset(&buf_out[32], 0, 6);
    buf_out[38] = buf_out[28];
    buf_out[39] = buf_out[29];
    buf_out[40] = buf_out[30];
    buf_out[41] = buf_out[31];

    autoip->state = SYN_AUTOIP_STATE_ANNOUNCE;
    autoip->announce_count++;
    *len_out = 60;

    return SYN_OK;
}

SYN_Status syn_autoip_process_arp(SYN_AUTOIP *autoip, SYN_ETH *eth, const uint8_t *arp_frame,
                                  size_t len)
{
    if (!autoip || !arp_frame || len < 42) {
        return SYN_INVALID_PARAM;
    }

    uint16_t ethertype = ((uint16_t)arp_frame[12] << 8) | arp_frame[13];
    if (ethertype != SYN_ETHTYPE_ARP) {
        return SYN_OK;
    }

    uint32_t sender_ip = ((uint32_t)arp_frame[28] << 24) | ((uint32_t)arp_frame[29] << 16) |
                         ((uint32_t)arp_frame[30] << 8) | arp_frame[31];

    /* Conflict check */
    if (sender_ip == autoip->ip_addr) {
        autoip->collisions++;
        /* Pick next candidate IP */
        uint8_t octet4 = (uint8_t)(autoip->ip_addr & 0xFF) + 1;
        if (octet4 == 0 || octet4 == 255)
            octet4 = 1;
        autoip->ip_addr = (autoip->ip_addr & 0xFFFFFF00UL) | octet4;
        autoip->state = SYN_AUTOIP_STATE_INIT;
        return SYN_BUSY;
    }

    /* No conflict detected -> transition to BOUND */
    autoip->state = SYN_AUTOIP_STATE_BOUND;
    if (eth) {
        eth->ip_addr = autoip->ip_addr;
        eth->netmask = SYN_AUTOIP_NETMASK;
        eth->gateway = 0;
    }

    return SYN_OK;
}
```


