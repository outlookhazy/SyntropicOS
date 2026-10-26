

# File syn\_autoip.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_autoip.h**](syn__autoip_8h.md)

[Go to the documentation of this file](syn__autoip_8h.md)


```C++

#ifndef SYN_AUTOIP_H
#define SYN_AUTOIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_AUTOIP_NETMASK 0xFFFF0000UL 
#define SYN_AUTOIP_BASE_IP 0xA9FE0101UL 
typedef enum {
    SYN_AUTOIP_STATE_INIT = 0, 
    SYN_AUTOIP_STATE_PROBE,    
    SYN_AUTOIP_STATE_ANNOUNCE, 
    SYN_AUTOIP_STATE_BOUND     
} SYN_AUTOIP_State;

typedef struct {
    SYN_AUTOIP_State state; 
    uint32_t ip_addr;       
    uint8_t probe_count;    
    uint8_t announce_count; 
    uint32_t collisions;    
} SYN_AUTOIP;

SYN_Status syn_autoip_init(SYN_AUTOIP *autoip, const uint8_t mac_addr[6]);

SYN_Status syn_autoip_build_probe(SYN_AUTOIP *autoip, const uint8_t mac_addr[6], uint8_t *buf_out,
                                  size_t *len_out);

SYN_Status syn_autoip_build_announce(SYN_AUTOIP *autoip, const uint8_t mac_addr[6],
                                     uint8_t *buf_out, size_t *len_out);

SYN_Status syn_autoip_process_arp(SYN_AUTOIP *autoip, SYN_ETH *eth, const uint8_t *arp_frame,
                                  size_t len);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

#define PT_AUTOIP_WAIT_BOUND(pt, autoip) \
    PT_WAIT_UNTIL(pt, (autoip)->state == SYN_AUTOIP_STATE_BOUND)

#define PT_AUTOIP_BLOCK_BOUND(pt, task, autoip) \
    PT_BLOCK_CONDITION(pt, task, (autoip)->state == SYN_AUTOIP_STATE_BOUND)

#ifdef __cplusplus
}
#endif

#endif /* SYN_AUTOIP_H */
```


