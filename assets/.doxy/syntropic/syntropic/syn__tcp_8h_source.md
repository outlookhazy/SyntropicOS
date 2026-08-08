

# File syn\_tcp.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tcp.h**](syn__tcp_8h.md)

[Go to the documentation of this file](syn__tcp_8h.md)


```C++

#ifndef SYN_TCP_H
#define SYN_TCP_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_eth.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ─────────────────────────────────────────────────────────── */

#ifndef SYN_TCP_MAX_CONNS
#define SYN_TCP_MAX_CONNS 1 
#endif

#ifndef SYN_TCP_BUF_SIZE
#define SYN_TCP_BUF_SIZE 512 
#endif

#define SYN_TCP_FLAG_FIN 0x01 
#define SYN_TCP_FLAG_SYN 0x02 
#define SYN_TCP_FLAG_RST 0x04 
#define SYN_TCP_FLAG_PSH 0x08 
#define SYN_TCP_FLAG_ACK 0x10 
/* ── States ────────────────────────────────────────────────────────────── */

typedef enum {
    SYN_TCP_CLOSED = 0,
    SYN_TCP_LISTEN = 1,
    SYN_TCP_SYN_RCVD = 2,
    SYN_TCP_ESTABLISHED = 3,
    SYN_TCP_FIN_WAIT = 4,
    SYN_TCP_CLOSE_WAIT = 5,
} SYN_TcpState;

/* ── TCP Header Format ─────────────────────────────────────────────────── */

typedef struct {
    uint16_t src_port;   
    uint16_t dst_port;   
    uint32_t seq_num;    
    uint32_t ack_num;    
    uint8_t data_offset; 
    uint8_t flags;       
    uint16_t window;     
    uint16_t checksum;   
    uint16_t urgent_ptr; 
} SYN_TcpHeader;

/* ── TCP Connection Block ──────────────────────────────────────────────── */

typedef struct SYN_TcpConn {
    SYN_TcpState state;    
    uint32_t local_ip;     
    uint32_t remote_ip;    
    uint16_t local_port;   
    uint16_t remote_port;  
    uint8_t remote_mac[6]; 
    uint32_t seq_nxt; 
    uint32_t ack_nxt; 
    uint8_t rx_buf[SYN_TCP_BUF_SIZE]; 
    uint16_t rx_len;                  
    uint8_t tx_buf[SYN_TCP_BUF_SIZE]; 
    uint16_t tx_len;                  
    SYN_Task *blocked_task; 
} SYN_TcpConn;

/* ── TCP Stack Container ───────────────────────────────────────────────── */

typedef struct {
    SYN_ETH *eth;                         
    SYN_TcpConn conns[SYN_TCP_MAX_CONNS]; 
} SYN_TCP;

/* ── Functions ─────────────────────────────────────────────────────────── */

SYN_Status syn_tcp_init(SYN_TCP *tcp, SYN_ETH *eth);

SYN_Status syn_tcp_listen(SYN_TCP *tcp, uint16_t port);

SYN_Status syn_tcp_process_packet(SYN_TCP *tcp, const uint8_t *ip_packet, size_t len,
                                  uint8_t *tx_out, size_t *tx_len);

uint16_t syn_tcp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *tcp_seg, size_t len);

/* ── Task Blocking Macros ───────────────────────────────────────────────── */

#define PT_TCP_BLOCK_READ(pt, task, conn, buf, max_len, bytes_read)                               \
    do {                                                                                          \
        (conn)->blocked_task = (task);                                                            \
        PT_BLOCK_CONDITION(pt, task, (conn)->rx_len > 0 || (conn)->state != SYN_TCP_ESTABLISHED); \
        if ((conn)->rx_len > 0) {                                                                 \
            size_t _n = ((conn)->rx_len < (max_len)) ? (conn)->rx_len : (max_len);                \
            memcpy((buf), (conn)->rx_buf, _n);                                                    \
            memmove((conn)->rx_buf, (conn)->rx_buf + _n, (conn)->rx_len - _n);                    \
            (conn)->rx_len -= _n;                                                                 \
            *(bytes_read) = _n;                                                                   \
        } else {                                                                                  \
            *(bytes_read) = 0;                                                                    \
        }                                                                                         \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_TCP_H */
```


