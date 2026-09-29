

# File syn\_eth.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_eth.h**](syn__eth_8h.md)

[Go to the documentation of this file](syn__eth_8h.md)


```C++

#ifndef SYN_ETH_H
#define SYN_ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_ETH_MAC_LEN 6U          
#define SYN_ETH_HEADER_LEN 14U      
#define SYN_ETH_MIN_FRAME_LEN 60U   
#define SYN_ETH_MAX_FRAME_LEN 1514U 
#ifndef SYN_ETH_ARP_CACHE_SIZE
#define SYN_ETH_ARP_CACHE_SIZE 8U 
#endif

#define SYN_ETHTYPE_IPV4 0x0800U     
#define SYN_ETHTYPE_ARP 0x0806U      
#define SYN_ETHTYPE_IPV6 0x86DDU     
#define SYN_ETHTYPE_ETHERCAT 0x88A4U 
#define SYN_ARP_OP_REQUEST 1U 
#define SYN_ARP_OP_REPLY 2U   
typedef struct {
    uint8_t dst_mac[SYN_ETH_MAC_LEN]; 
    uint8_t src_mac[SYN_ETH_MAC_LEN]; 
    uint16_t ethertype;               
} SYN_ETH_Header;

typedef struct {
    uint32_t ip;                  
    uint8_t mac[SYN_ETH_MAC_LEN]; 
    uint32_t last_seen_ms;        
    bool valid;                   
} SYN_ETH_ArpEntry;

typedef struct {
    uint8_t mac_addr[SYN_ETH_MAC_LEN];                  
    uint32_t ip_addr;                                   
    uint32_t netmask;                                   
    uint32_t gateway;                                   
    SYN_ETH_ArpEntry arp_cache[SYN_ETH_ARP_CACHE_SIZE]; 
    uint32_t frames_tx;                                 
    uint32_t frames_rx;                                 
    uint32_t arp_requests;                              
    uint32_t arp_replies;                               
} SYN_ETH;

SYN_Status syn_eth_generate_mac(const void *uid_bytes, size_t uid_len, uint8_t mac_out[6]);

SYN_Status syn_eth_init(SYN_ETH *eth, const uint8_t mac_addr[6], uint32_t ip_addr);

SYN_Status syn_eth_process_frame(SYN_ETH *eth, const uint8_t *frame, size_t len, uint8_t *tx_buf,
                                 size_t *tx_len);

SYN_Status syn_eth_build_frame(SYN_ETH *eth, const uint8_t dst_mac[6], uint16_t ethertype,
                               const uint8_t *payload, size_t payload_len, uint8_t *frame_out,
                               size_t *frame_len);

SYN_Status syn_eth_arp_lookup(SYN_ETH *eth, uint32_t ip, uint8_t mac_out[6]);

SYN_Status syn_eth_arp_update(SYN_ETH *eth, uint32_t ip, const uint8_t mac[6]);

/* ── Network Frame Serialization Helpers ───────────────────────────────── */

uint16_t syn_ip_checksum(const void *buf, size_t len);

size_t syn_eth_pack_header(uint8_t *buf, size_t buf_size, const uint8_t dst_mac[6],
                           const uint8_t src_mac[6], uint16_t ethertype);

size_t syn_ip_pack_header(uint8_t *buf, size_t buf_size, uint32_t src_ip, uint32_t dst_ip,
                          uint8_t proto, uint16_t payload_len, uint16_t id);

/* ── Protocol engine injection ──────────────────────────────────────────── */

struct SYN_ICMP *syn_eth_get_icmp_instance(void);

struct SYN_TCP *syn_eth_get_tcp_instance(void);

/* ── Protothread Coroutine Integration ──────────────────────────────────── */

#include "port/syn_port_eth.h"
#include "syntropic/pt/syn_pt.h"

#define PT_ETH_WAIT_FRAME(pt, rx_buf, max_len, rx_len) \
    PT_WAIT_UNTIL(pt, syn_port_eth_rx((rx_buf), (max_len), (rx_len)) == SYN_OK)

#define PT_ETH_WAIT_ARP(pt, eth, target_ip, mac_out) \
    PT_WAIT_UNTIL(pt, syn_eth_arp_lookup((eth), (target_ip), (mac_out)) == SYN_OK)

#ifdef __cplusplus
}
#endif

#endif /* SYN_ETH_H */
```


