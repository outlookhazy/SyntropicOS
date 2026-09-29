

# File syn\_dhcp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dhcp.h**](syn__dhcp_8h.md)

[Go to the documentation of this file](syn__dhcp_8h.md)


```C++

#ifndef SYN_DHCP_H
#define SYN_DHCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_DHCP_CLIENT_PORT 68U 
#define SYN_DHCP_SERVER_PORT 67U 
#define SYN_DHCP_MAGIC_COOKIE 0x63825363UL 
#define SYN_DHCP_DISCOVER 1U 
#define SYN_DHCP_OFFER 2U    
#define SYN_DHCP_REQUEST 3U  
#define SYN_DHCP_DECLINE 4U  
#define SYN_DHCP_ACK 5U      
#define SYN_DHCP_NAK 6U      
#define SYN_DHCP_OPT_SUBNET_MASK 1U 
#define SYN_DHCP_OPT_ROUTER 3U      
#define SYN_DHCP_OPT_LEASE_TIME 51U 
#define SYN_DHCP_OPT_MSG_TYPE 53U   
#define SYN_DHCP_OPT_END 255U       
typedef enum {
    SYN_DHCP_STATE_INIT = 0,
    SYN_DHCP_STATE_DISCOVER,
    SYN_DHCP_STATE_OFFER,
    SYN_DHCP_STATE_REQUEST,
    SYN_DHCP_STATE_BOUND
} SYN_DHCP_State;

typedef struct {
    SYN_DHCP_State state;    
    uint32_t xid;            
    uint32_t offered_ip;     
    uint32_t subnet_mask;    
    uint32_t gateway;        
    uint32_t lease_time_sec; 
    uint32_t server_ip;      
    uint32_t discovers_sent; 
    uint32_t requests_sent;  
    uint32_t acks_received;  
} SYN_DHCP;

SYN_Status syn_dhcp_init(SYN_DHCP *dhcp, uint32_t xid);

SYN_Status syn_dhcp_build_discover(SYN_DHCP *dhcp, const uint8_t mac_addr[6], uint8_t *buf_out,
                                   size_t buf_size, size_t *len_out);

SYN_Status syn_dhcp_build_request(SYN_DHCP *dhcp, const uint8_t mac_addr[6], uint8_t *buf_out,
                                  size_t buf_size, size_t *len_out);

SYN_Status syn_dhcp_process_packet(SYN_DHCP *dhcp, SYN_ETH *eth, const uint8_t *dhcp_pkt,
                                   size_t len);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

#define PT_DHCP_WAIT_BOUND(pt, dhcp) PT_WAIT_UNTIL(pt, (dhcp)->state == SYN_DHCP_STATE_BOUND)

#define PT_DHCP_BLOCK_BOUND(pt, task, dhcp) \
    PT_BLOCK_CONDITION(pt, task, (dhcp)->state == SYN_DHCP_STATE_BOUND)

#ifdef __cplusplus
}
#endif

#endif /* SYN_DHCP_H */
```


