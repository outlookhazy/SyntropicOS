

# File syn\_netcfg.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netcfg.c**](syn__netcfg_8c.md)

[Go to the documentation of this file](syn__netcfg_8c.md)


```C++

#include "syntropic/net/syn_netcfg.h"

#include <string.h>

SYN_Status syn_netcfg_init(SYN_NETCFG *netcfg, SYN_NETCFG_Mode mode, const uint8_t mac[6])
{
    if (!netcfg || !mac) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }

    memset(netcfg, 0, sizeof(*netcfg));
    netcfg->mode = mode;
    netcfg->link_state = SYN_NETCFG_LINK_UP;

    syn_dhcp_init(&netcfg->dhcp, 0x12345678UL);
    syn_autoip_init(&netcfg->autoip, mac);

    if (mode == SYN_NETCFG_MODE_STATIC) {
        netcfg->is_bound = true;
    }

    return SYN_OK;
}

SYN_Status syn_netcfg_set_link_callback(SYN_NETCFG *netcfg, SYN_NETCFG_LinkCb cb, void *user_data)
{
    if (!netcfg) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }
    netcfg->link_cb = cb;
    netcfg->user_data = user_data;
    return SYN_OK;
}

SYN_Status syn_netcfg_set_link_state(SYN_NETCFG *netcfg, SYN_ETH *eth, SYN_NETCFG_LinkState state)
{
    if (!netcfg || !eth) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }

    netcfg->link_state = state;

    if (state == SYN_NETCFG_LINK_DOWN) {
        netcfg->is_bound = false;
        netcfg->assigned_ip = 0;
        netcfg->assigned_netmask = 0;
        netcfg->assigned_gateway = 0;

        eth->ip_addr = 0;
        eth->netmask = 0;
        eth->gateway = 0;

        netcfg->dhcp.state = SYN_DHCP_STATE_INIT;
        netcfg->autoip.state = SYN_AUTOIP_STATE_INIT;
    } else if (state == SYN_NETCFG_LINK_UP) {
        if (netcfg->mode == SYN_NETCFG_MODE_STATIC) {
            netcfg->assigned_ip = netcfg->static_ip;
            netcfg->assigned_netmask = netcfg->static_netmask;
            netcfg->assigned_gateway = netcfg->static_gateway;
            netcfg->is_bound = (netcfg->static_ip != 0);

            eth->ip_addr = netcfg->static_ip;
            eth->netmask = netcfg->static_netmask;
            eth->gateway = netcfg->static_gateway;
        } else {
            netcfg->is_bound = false;
            netcfg->dhcp.state = SYN_DHCP_STATE_INIT;
            netcfg->autoip.state = SYN_AUTOIP_STATE_INIT;
        }
    }

    if (netcfg->link_cb) {
        netcfg->link_cb(netcfg, state, netcfg->user_data);
    }

    return SYN_OK;
}

SYN_Status syn_netcfg_set_static(SYN_NETCFG *netcfg, SYN_ETH *eth, uint32_t ip, uint32_t netmask,
                                 uint32_t gateway)
{
    if (!netcfg || !eth || ip == 0) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }

    netcfg->mode = SYN_NETCFG_MODE_STATIC;
    netcfg->static_ip = ip;
    netcfg->static_netmask = netmask;
    netcfg->static_gateway = gateway;
    netcfg->assigned_ip = ip;
    netcfg->assigned_netmask = netmask;
    netcfg->assigned_gateway = gateway;
    netcfg->is_bound = (netcfg->link_state == SYN_NETCFG_LINK_UP);

    if (netcfg->is_bound) {
        eth->ip_addr = ip;
        eth->netmask = netmask;
        eth->gateway = gateway;
    }

    return SYN_OK;
}

SYN_Status syn_netcfg_trigger_autoip_fallback(SYN_NETCFG *netcfg, SYN_ETH *eth,
                                              const uint8_t mac[6])
{
    if (!netcfg || !eth || !mac) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter
                                     fallback */
    }

    syn_autoip_init(&netcfg->autoip, mac);
    netcfg->autoip.state = SYN_AUTOIP_STATE_BOUND;

    netcfg->assigned_ip = netcfg->autoip.ip_addr;
    netcfg->assigned_netmask = SYN_AUTOIP_NETMASK;
    netcfg->assigned_gateway = 0;
    netcfg->is_bound = (netcfg->link_state == SYN_NETCFG_LINK_UP);

    if (netcfg->is_bound) {
        eth->ip_addr = netcfg->assigned_ip;
        eth->netmask = netcfg->assigned_netmask;
        eth->gateway = 0;
    }

    return SYN_OK;
}
```


