

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
#include <string.h>

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

#ifndef SYN_UDP_MAX_DGRAMS
#define SYN_UDP_MAX_DGRAMS 8U 
#endif

typedef struct {
    uint16_t src_port; 
    uint16_t dst_port; 
    uint16_t length;   
    uint16_t checksum; 
} SYN_UdpHeader;

typedef struct {
    uint32_t src_ip;   
    uint16_t src_port; 
    uint16_t len;      
} SYN_UdpDatagram;

/* ── UDP Socket Slot ────────────────────────────────────────────────────── */

typedef struct {
    bool is_bound;        
    uint16_t local_port;  
    uint32_t remote_ip;   
    uint16_t remote_port; 

    uint8_t rx_buf[SYN_UDP_BUF_SIZE];           
    uint16_t rx_len;                            
    uint16_t rx_total_len;                      
    SYN_UdpDatagram dgrams[SYN_UDP_MAX_DGRAMS]; 
    uint8_t dgram_head;                         
    uint8_t dgram_count;                        

    SYN_Task *blocked_task; 
} SYN_UdpSocket;

/* ── UDP Stack Container ───────────────────────────────────────────────── */

typedef struct {
    SYN_ETH *eth;                               
    SYN_UdpSocket sockets[SYN_UDP_MAX_SOCKETS]; 
    uint16_t next_ephemeral_port;               
} SYN_UDP;

/* ── Functions ─────────────────────────────────────────────────────────── */

SYN_Status syn_udp_init(SYN_UDP *udp, SYN_ETH *eth);

SYN_UdpSocket *syn_udp_bind(SYN_UDP *udp, uint16_t local_port);

void syn_udp_unbind(SYN_UdpSocket *sock);

SYN_Status syn_udp_process_packet(SYN_UDP *udp, const uint8_t *ip_packet, size_t len);

int syn_udp_sendto(SYN_UDP *udp, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                   const uint8_t *data, size_t data_len, uint8_t *tx_out, size_t *tx_len);

uint16_t syn_udp_checksum(uint32_t src_ip, uint32_t dst_ip, const uint8_t *udp_seg, size_t len);

SYN_Status syn_udp_recv(SYN_UdpSocket *sock, void *buf, size_t max_len, size_t *bytes_read,
                        uint32_t *src_ip, uint16_t *src_port);

static inline bool syn_udp_pt_args_valid(SYN_PT *pt, const SYN_Task *task,
                                         const SYN_UdpSocket *sock, const void *buf,
                                         const size_t *bytes_read)
{
    if (pt == NULL || task == NULL || sock == NULL || buf == NULL || bytes_read == NULL) {
        if (pt != NULL) {
            PT_INIT(pt);
        }
        return false;
    }
    return true;
}

/* ── Task Blocking Macros ───────────────────────────────────────────────── */

#define PT_UDP_BLOCK_READ(pt, task, sock, buf, max_len, bytes_read, src_ip, src_port)     \
    do {                                                                                  \
        while (1) {                                                                       \
            if (!syn_udp_pt_args_valid((pt), (task), (sock), (buf), (bytes_read))) {      \
                return PT_EXITED;                                                         \
            }                                                                             \
            (sock)->blocked_task = (task);                                                \
            syn_port_enter_critical();                                                    \
            if ((task)->state == (uint8_t)SYN_TASK_READY) {                               \
                (task)->state = (uint8_t)SYN_TASK_BLOCKED;                                \
                SYN_PT_STATE_BARRIER();                                                   \
            }                                                                             \
            syn_port_exit_critical();                                                     \
            if ((sock)->dgram_count > 0U || (sock)->rx_len > 0U || !(sock)->is_bound) {   \
                syn_port_enter_critical();                                                \
                if ((task)->state == (uint8_t)SYN_TASK_BLOCKED) {                         \
                    (task)->state = (uint8_t)SYN_TASK_READY;                              \
                }                                                                         \
                syn_port_exit_critical();                                                 \
                break;                                                                    \
            }                                                                             \
            PT_PASS(pt);                                                                  \
        }                                                                                 \
        (sock)->blocked_task = NULL;                                                      \
        (void)syn_udp_recv((sock), (buf), (max_len), (bytes_read), (src_ip), (src_port)); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDP_H */
```


