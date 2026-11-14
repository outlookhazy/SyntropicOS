

# File syn\_dhcp.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dhcp.c**](syn__dhcp_8c.md)

[Go to the documentation of this file](syn__dhcp_8c.md)


```C++


#include "syntropic/net/syn_dhcp.h"

#include <string.h>

SYN_Status syn_dhcp_init(SYN_DHCP *dhcp, uint32_t xid)
{
    if (!dhcp) {
        return SYN_INVALID_PARAM;
    }
    memset(dhcp, 0, sizeof(*dhcp));
    dhcp->state = SYN_DHCP_STATE_INIT;
    dhcp->xid = (xid != 0) ? xid : 0x12345678UL;
    return SYN_OK;
}

static void pack_uint32(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val);
}

static uint32_t unpack_uint32(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) |
           (uint32_t)buf[3];
}

SYN_Status syn_dhcp_build_discover(SYN_DHCP *dhcp, const uint8_t mac_addr[6], uint8_t *buf_out,
                                   size_t buf_size, size_t *len_out)
{
    if (!dhcp || !mac_addr || !buf_out || buf_size < 250U || !len_out) {
        return SYN_INVALID_PARAM;
    }

    memset(buf_out, 0, 250);
    buf_out[0] = 1; /* op = Boot Request */
    buf_out[1] = 1; /* htype = Ethernet */
    buf_out[2] = 6; /* hlen = 6 */
    buf_out[3] = 0; /* hops = 0 */

    pack_uint32(&buf_out[4], dhcp->xid);
    memcpy(&buf_out[28], mac_addr, 6); /* chaddr */

    /* Magic Cookie: 0x63, 0x82, 0x53, 0x63 */
    pack_uint32(&buf_out[236], SYN_DHCP_MAGIC_COOKIE);

    /* Options: Option 53 = DHCPDISCOVER (1), Option 255 = END */
    size_t opt_idx = 240;
    buf_out[opt_idx++] = SYN_DHCP_OPT_MSG_TYPE;
    buf_out[opt_idx++] = 1;
    buf_out[opt_idx++] = SYN_DHCP_DISCOVER;
    buf_out[opt_idx++] = SYN_DHCP_OPT_END;

    dhcp->state = SYN_DHCP_STATE_DISCOVER;
    dhcp->discovers_sent++;
    *len_out = opt_idx;

    return SYN_OK;
}

SYN_Status syn_dhcp_build_request(SYN_DHCP *dhcp, const uint8_t mac_addr[6], uint8_t *buf_out,
                                  size_t buf_size, size_t *len_out)
{
    if (!dhcp || !mac_addr || !buf_out || buf_size < 260U || !len_out) {
        return SYN_INVALID_PARAM;
    }

    memset(buf_out, 0, 260);
    buf_out[0] = 1; /* op = Boot Request */
    buf_out[1] = 1; /* htype = Ethernet */
    buf_out[2] = 6; /* hlen = 6 */
    buf_out[3] = 0;

    pack_uint32(&buf_out[4], dhcp->xid);
    memcpy(&buf_out[28], mac_addr, 6);

    /* Magic Cookie */
    pack_uint32(&buf_out[236], SYN_DHCP_MAGIC_COOKIE);

    size_t opt_idx = 240;
    /* Option 53 = DHCPREQUEST (3) */
    buf_out[opt_idx++] = SYN_DHCP_OPT_MSG_TYPE;
    buf_out[opt_idx++] = 1;
    buf_out[opt_idx++] = SYN_DHCP_REQUEST;

    /* Option 50 = Requested IP Address */
    buf_out[opt_idx++] = 50;
    buf_out[opt_idx++] = 4;
    pack_uint32(&buf_out[opt_idx], dhcp->offered_ip);
    opt_idx += 4;

    /* Option 54 = Server Identifier */
    if (dhcp->server_ip != 0) {
        buf_out[opt_idx++] = 54;
        buf_out[opt_idx++] = 4;
        pack_uint32(&buf_out[opt_idx], dhcp->server_ip);
        opt_idx += 4;
    }

    buf_out[opt_idx++] = SYN_DHCP_OPT_END;

    dhcp->state = SYN_DHCP_STATE_REQUEST;
    dhcp->requests_sent++;
    *len_out = opt_idx;

    return SYN_OK;
}

typedef struct {
    uint8_t msg_type; 
    uint32_t subnet;  
    uint32_t router;  
    uint32_t lease;   
    uint32_t server;  
} SYN_DhcpParsedOpts;

static bool dhcp_reply_valid(const SYN_DHCP *dhcp, const uint8_t *dhcp_pkt, size_t len)
{
    if (!dhcp || !dhcp_pkt || len < 244U || dhcp_pkt[0] != 2U) {
        return false;
    }
    if (unpack_uint32(&dhcp_pkt[236]) != SYN_DHCP_MAGIC_COOKIE) {
        return false;
    }
    return unpack_uint32(&dhcp_pkt[4]) == dhcp->xid;
}

static void dhcp_apply_option(SYN_DhcpParsedOpts *opts, uint8_t opt, uint8_t opt_len,
                              const uint8_t *val)
{
    if (opt == SYN_DHCP_OPT_MSG_TYPE && opt_len >= 1U) {
        opts->msg_type = val[0];
    } else if (opt == SYN_DHCP_OPT_SUBNET_MASK && opt_len >= 4U) {
        opts->subnet = unpack_uint32(val);
    } else if (opt == SYN_DHCP_OPT_ROUTER && opt_len >= 4U) {
        opts->router = unpack_uint32(val);
    } else if (opt == SYN_DHCP_OPT_LEASE_TIME && opt_len >= 4U) {
        opts->lease = unpack_uint32(val);
    } else if (opt == 54U && opt_len >= 4U) {
        opts->server = unpack_uint32(val);
    }
}

static SYN_Status dhcp_apply_message(SYN_DHCP *dhcp, SYN_ETH *eth, uint32_t yiaddr,
                                     const SYN_DhcpParsedOpts *opts)
{
    if (opts->msg_type == SYN_DHCP_OFFER) {
        dhcp->offered_ip = yiaddr;
        dhcp->subnet_mask = opts->subnet;
        dhcp->gateway = opts->router;
        dhcp->lease_time_sec = opts->lease;
        dhcp->server_ip = opts->server;
        dhcp->state = SYN_DHCP_STATE_OFFER;
        return SYN_BUSY;
    }

    if (opts->msg_type == SYN_DHCP_ACK) {
        if (yiaddr == 0U) {
            return SYN_INVALID_PARAM;
        }
        dhcp->offered_ip = yiaddr;
        dhcp->subnet_mask = opts->subnet;
        dhcp->gateway = opts->router;
        dhcp->lease_time_sec = opts->lease;
        dhcp->state = SYN_DHCP_STATE_BOUND;
        dhcp->acks_received++;
        if (eth != NULL) {
            eth->ip_addr = yiaddr;
            eth->netmask = opts->subnet;
            eth->gateway = opts->router;
        }
        return SYN_OK;
    }

    if (opts->msg_type == SYN_DHCP_NAK) {
        dhcp->state = SYN_DHCP_STATE_INIT;
        dhcp->offered_ip = 0U;
        dhcp->subnet_mask = 0U;
        dhcp->gateway = 0U;
        dhcp->lease_time_sec = 0U;
        if (eth != NULL) {
            eth->ip_addr = 0U;
            eth->netmask = 0U;
            eth->gateway = 0U;
        }
    }

    return SYN_BUSY;
}

SYN_Status syn_dhcp_process_packet(SYN_DHCP *dhcp, SYN_ETH *eth, const uint8_t *dhcp_pkt,
                                   size_t len)
{
    if (!dhcp_reply_valid(dhcp, dhcp_pkt, len)) {
        return SYN_INVALID_PARAM;
    }

    uint32_t yiaddr = unpack_uint32(&dhcp_pkt[16]);
    SYN_DhcpParsedOpts opts = {0};

    size_t i = 240U;
    while (i < len && dhcp_pkt[i] != SYN_DHCP_OPT_END) {
        uint8_t opt = dhcp_pkt[i++];
        if (opt == 0U) {
            continue; /* Pad */
        }
        if (i >= len) {
            break;
        }
        uint8_t opt_len = dhcp_pkt[i++];
        if (i + opt_len > len) {
            break;
        }
        dhcp_apply_option(&opts, opt, opt_len, &dhcp_pkt[i]);
        i += opt_len;
    }

    return dhcp_apply_message(dhcp, eth, yiaddr, &opts);
}
```


