

# File syn\_lintp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lintp.h**](syn__lintp_8h.md)

[Go to the documentation of this file](syn__lintp_8h.md)


```C++

#ifndef SYN_LINTP_H
#define SYN_LINTP_H

#include "../common/syn_defs.h"

#include <stdint.h>

#if !defined(_SSIZE_T_DEFINED_) && !defined(_SSIZE_T_DECLARED) && !defined(__ssize_t_defined) && \
    !defined(_SSIZE_T_) && !defined(_SSIZE_T) && !defined(_SSIZE_T_DEFINED)
typedef intptr_t ssize_t;
#define _SSIZE_T_DEFINED_
#define _SSIZE_T_DECLARED
#define __ssize_t_defined
#define _SSIZE_T_
#define _SSIZE_T
#define _SSIZE_T_DEFINED
#endif
#if !defined(SYN_USE_LINTP) || SYN_USE_LINTP

#ifdef __cplusplus
extern "C" {
#endif

/* ── LIN Diagnostic Addressing Constants ─────────────────────────────────── */

#define SYN_LIN_ID_MASTER_REQ 0x3C 
#define SYN_LIN_ID_SLAVE_RESP 0x3D 
#define SYN_LINTP_NAD_FUNCTIONAL 0x7E 
#define SYN_LINTP_NAD_BROADCAST 0x7F  
/* ── LIN TP PCI Frame Types ─────────────────────────────────────────────── */

#define SYN_LINTP_PCI_SF 0x00 
#define SYN_LINTP_PCI_FF 0x01 
#define SYN_LINTP_PCI_CF 0x02 
/* ── LIN TP Link State Machine Definitions ───────────────────────────────── */

typedef enum {
    SYN_LINTP_STATE_IDLE = 0,
    SYN_LINTP_STATE_TRANSMITTING_FF,
    SYN_LINTP_STATE_TRANSMITTING_CF,
    SYN_LINTP_STATE_RECEIVING_CF
} SYN_LINTP_State;

typedef struct {
    uint8_t nad;          
    uint8_t padding_byte; 
    /* RX State */
    SYN_LINTP_State rx_state; 
    uint8_t *rx_buf;          
    size_t rx_buf_size;       
    size_t rx_total_len;      
    size_t rx_offset;         
    uint8_t rx_sn;            
    uint8_t rx_nad;           
    bool rx_completed;        
    /* TX State */
    SYN_LINTP_State tx_state; 
    const uint8_t *tx_buf;    
    size_t tx_total_len;      
    size_t tx_offset;         
    uint8_t tx_sn;            
    uint8_t tx_nad;           
    bool tx_frame_pending;    
    uint8_t tx_frame[8];      
    /* Timers & Timeouts */
    uint32_t timer_n_as_ms; 
    uint32_t timer_n_cr_ms; 
    uint32_t rx_timer_ms;   
    uint32_t tx_timer_ms;   
} SYN_LINTP_Link;

/* ── Function Contracts ─────────────────────────────────────────────────── */

void syn_lintp_init(SYN_LINTP_Link *link, uint8_t nad, uint8_t *rx_buf, size_t rx_size,
                    uint8_t *tx_buf, size_t tx_size);

void syn_lintp_set_timeouts(SYN_LINTP_Link *link, uint32_t n_as_ms, uint32_t n_cr_ms);

void syn_lintp_set_padding(SYN_LINTP_Link *link, uint8_t pad_byte);

SYN_Status syn_lintp_send(SYN_LINTP_Link *link, uint8_t nad, const uint8_t *payload, size_t len);

bool syn_lintp_get_tx_frame(SYN_LINTP_Link *link, uint8_t out_frame[8]);

void syn_lintp_process_rx_frame(SYN_LINTP_Link *link, const uint8_t frame[8]);

ssize_t syn_lintp_receive(SYN_LINTP_Link *link, uint8_t *out_buf, size_t max_len);

void syn_lintp_step(SYN_LINTP_Link *link, uint32_t dt_ms);

bool syn_lintp_is_tx_idle(const SYN_LINTP_Link *link);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LINTP */
#endif /* SYN_LINTP_H */
```


