

# File syn\_netcfg.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netcfg.h**](syn__netcfg_8h.md)

[Go to the documentation of this file](syn__netcfg_8h.md)


```C++

#ifndef SYN_NETCFG_H
#define SYN_NETCFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_autoip.h"
#include "syntropic/net/syn_dhcp.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SYN_NETCFG_MODE_STATIC = 0,
    SYN_NETCFG_MODE_DHCP,
    SYN_NETCFG_MODE_AUTO /* DHCP with RFC 3927 AutoIP fallback */
} SYN_NETCFG_Mode;

typedef enum { SYN_NETCFG_LINK_DOWN = 0, SYN_NETCFG_LINK_UP } SYN_NETCFG_LinkState;

struct SYN_NETCFG_s;

typedef void (*SYN_NETCFG_LinkCb)(struct SYN_NETCFG_s *netcfg, SYN_NETCFG_LinkState state,
                                  void *user_data);

typedef struct SYN_NETCFG_s {
    SYN_NETCFG_Mode mode;            
    SYN_NETCFG_LinkState link_state; 
    SYN_DHCP dhcp;                   
    SYN_AUTOIP autoip;               
    bool is_bound;                   
    uint32_t static_ip;              
    uint32_t static_netmask;         
    uint32_t static_gateway;         
    uint32_t assigned_ip;            
    uint32_t assigned_netmask;       
    uint32_t assigned_gateway;       
    SYN_NETCFG_LinkCb link_cb;       
    void *user_data;                 
} SYN_NETCFG;

SYN_Status syn_netcfg_init(SYN_NETCFG *netcfg, SYN_NETCFG_Mode mode, const uint8_t mac[6]);

SYN_Status syn_netcfg_set_link_callback(SYN_NETCFG *netcfg, SYN_NETCFG_LinkCb cb, void *user_data);

SYN_Status syn_netcfg_set_link_state(SYN_NETCFG *netcfg, SYN_ETH *eth, SYN_NETCFG_LinkState state);

SYN_Status syn_netcfg_set_static(SYN_NETCFG *netcfg, SYN_ETH *eth, uint32_t ip, uint32_t netmask,
                                 uint32_t gateway);

SYN_Status syn_netcfg_trigger_autoip_fallback(SYN_NETCFG *netcfg, SYN_ETH *eth,
                                              const uint8_t mac[6]);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

#define PT_NETCFG_WAIT_BOUND(pt, netcfg) PT_WAIT_UNTIL(pt, (netcfg)->is_bound == true)

#define PT_NETCFG_BLOCK_BOUND(pt, task, netcfg) \
    PT_BLOCK_CONDITION(pt, task, (netcfg)->is_bound == true)

#define PT_NETCFG_WAIT_LINK_UP(pt, netcfg) \
    PT_WAIT_UNTIL(pt, (netcfg)->link_state == SYN_NETCFG_LINK_UP)

#define PT_NETCFG_BLOCK_LINK_UP(pt, task, netcfg) \
    PT_BLOCK_CONDITION(pt, task, (netcfg)->link_state == SYN_NETCFG_LINK_UP)

#ifdef __cplusplus
}
#endif

#endif /* SYN_NETCFG_H */
```


