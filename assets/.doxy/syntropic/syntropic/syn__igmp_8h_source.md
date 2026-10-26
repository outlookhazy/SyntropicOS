

# File syn\_igmp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_igmp.h**](syn__igmp_8h.md)

[Go to the documentation of this file](syn__igmp_8h.md)


```C++

#ifndef SYN_IGMP_H
#define SYN_IGMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_IGMP_TYPE_MEMBERSHIP_QUERY 0x11U 
#define SYN_IGMP_TYPE_V2_REPORT 0x16U        
#define SYN_IGMP_TYPE_V2_LEAVE 0x17U         
#define SYN_IGMP_MAX_GROUPS 4U 
typedef struct {
    uint32_t group_ip; 
    bool joined;       
} SYN_IGMP_Group;

typedef struct {
    SYN_IGMP_Group groups[SYN_IGMP_MAX_GROUPS]; 
    uint32_t reports_sent;                      
    uint32_t queries_received;                  
    uint32_t leaves_sent;                       
} SYN_IGMP;

SYN_Status syn_igmp_init(SYN_IGMP *igmp);

SYN_Status syn_igmp_build_report(SYN_IGMP *igmp, SYN_ETH *eth, uint8_t type, uint32_t group_ip,
                                 uint8_t *frame_out, size_t *frame_len);

SYN_Status syn_igmp_join_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                               size_t *frame_len);

SYN_Status syn_igmp_leave_group(SYN_IGMP *igmp, SYN_ETH *eth, uint32_t group_ip, uint8_t *frame_out,
                                size_t *frame_len);

SYN_Status syn_igmp_process_packet(SYN_IGMP *igmp, SYN_ETH *eth, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IGMP_H */
```


