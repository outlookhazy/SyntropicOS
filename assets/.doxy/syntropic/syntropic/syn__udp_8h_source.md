

# File syn\_udp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_udp.h**](syn__udp_8h.md)

[Go to the documentation of this file](syn__udp_8h.md)


```C++

#ifndef SYN_UDP_H
#define SYN_UDP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ─────────────────────────────────────────────────────────── */

#ifndef SYN_UDP_MAX_SOCKETS
#define SYN_UDP_MAX_SOCKETS 4 
#endif

#ifndef SYN_UDP_BUF_SIZE
#define SYN_UDP_BUF_SIZE 256 
#endif

typedef struct {
    uint16_t src_port; 
    uint16_t dst_port; 
    uint16_t length;   
    uint16_t checksum; 
} SYN_UdpHeader;

/* ── UDP Socket Slot ────────────────────────────────────────────────────── */

typedef struct {
    bool is_bound;        
    uint16_t local_port;  
    uint32_t remote_ip;   
    uint16_t remote_port; 
    uint8_t rx_buf[SYN_UDP_BUF_SIZE]; 
    uint16_t rx_len;                  
    SYN_Task *blocked_task; 
} SYN_UdpSocket;

/* ── UDP Stack Container ───────────────────────────────────────────────── */

typedef struct {
    SYN_ETH *eth;                               
    SYN_UdpSocket sockets[SYN_UDP_MAX_SOCKETS]; 
} SYN_UDP;

/* ── Functions ─────────────────────────────────────────────────────────── */

SYN_Status syn_udp_init(SYN_UDP *udp, SYN_ETH *eth);

SYN_UdpSocket *syn_udp_bind(SYN_UDP *udp, uint16_t local_port);

void syn_udp_unbind(SYN_UdpSocket *sock);

SYN_Status syn_udp_process_packet(SYN_UDP *udp, const uint8_t *ip_packet, size_t len);

int syn_udp_sendto(SYN_UDP *udp, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                   const uint8_t *data, size_t data_len, uint8_t *tx_out, size_t *tx_len);

uint16_t syn_udp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *udp_seg, size_t len);

/* ── Task Blocking Macros ───────────────────────────────────────────────── */

#define PT_UDP_BLOCK_READ(pt, task, sock, buf, max_len, bytes_read, src_ip, src_port) \
    do {                                                                              \
        (sock)->blocked_task = (task);                                                \
        PT_BLOCK_CONDITION(pt, task, (sock)->rx_len > 0 || !(sock)->is_bound);        \
        if ((sock)->rx_len > 0) {                                                     \
            size_t _n = ((sock)->rx_len < (max_len)) ? (sock)->rx_len : (max_len);    \
            memcpy((buf), (sock)->rx_buf, _n);                                        \
            memmove((sock)->rx_buf, (sock)->rx_buf + _n, (sock)->rx_len - _n);        \
            (sock)->rx_len -= _n;                                                     \
            *(bytes_read) = _n;                                                       \
            if (src_ip)                                                               \
                *(src_ip) = (sock)->remote_ip;                                        \
            if (src_port)                                                             \
                *(src_port) = (sock)->remote_port;                                    \
        } else {                                                                      \
            *(bytes_read) = 0;                                                        \
        }                                                                             \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDP_H */
```


