

# File syn\_icmp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_icmp.h**](syn__icmp_8h.md)

[Go to the documentation of this file](syn__icmp_8h.md)


```C++

#ifndef SYN_ICMP_H
#define SYN_ICMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_ICMP_TYPE_ECHO_REPLY 0U   
#define SYN_ICMP_TYPE_UNREACHABLE 3U  
#define SYN_ICMP_TYPE_ECHO_REQUEST 8U 
#define SYN_ICMP_HEADER_LEN 8U 
typedef struct {
    uint32_t echo_requests_rx; 
    uint32_t echo_replies_tx;  
    uint32_t echo_requests_tx; 
    uint32_t echo_replies_rx;  
    uint32_t errors;           
} SYN_ICMP;

SYN_Status syn_icmp_init(SYN_ICMP *icmp);

uint16_t syn_icmp_checksum(const void *buf, size_t len);

SYN_Status syn_icmp_process_packet(SYN_ICMP *icmp, const uint8_t *ip_pkt, size_t len,
                                   uint8_t *frame_tx, size_t *tx_len);

SYN_Status syn_icmp_build_echo_request(SYN_ICMP *icmp, SYN_ETH *eth, uint32_t dst_ip,
                                       const uint8_t dst_mac[6], uint16_t id, uint16_t seq,
                                       const uint8_t *payload, size_t payload_len,
                                       uint8_t *frame_out, size_t *frame_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ICMP_H */
```


