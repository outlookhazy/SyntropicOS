

# File syn\_isotp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_isotp.h**](syn__isotp_8h.md)

[Go to the documentation of this file](syn__isotp_8h.md)


```C++

#ifndef SYN_ISOTP_H
#define SYN_ISOTP_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#if defined(__AVR__) && !defined(_SSIZE_T_DEFINED_)
typedef int ssize_t;
#define _SSIZE_T_DEFINED_
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
#define SYN_ISOTP_MAX_PAYLOAD             0xFFFFFFFFU 
#else
#define SYN_ISOTP_MAX_PAYLOAD             4095U       
#endif

#define SYN_ISOTP_PCI_SF                  0x00U 
#define SYN_ISOTP_PCI_FF                  0x10U 
#define SYN_ISOTP_PCI_CF                  0x20U 
#define SYN_ISOTP_PCI_FC                  0x30U 
#define SYN_ISOTP_DEFAULT_N_BS_MS         1000U 
#define SYN_ISOTP_DEFAULT_N_CR_MS         1000U 
#define SYN_ISOTP_FC_CTS                  0x00U 
#define SYN_ISOTP_FC_WAIT                 0x01U 
#define SYN_ISOTP_FC_OVERFLOW             0x02U 
typedef enum {
    SYN_ISOTP_TX_IDLE = 0,
    SYN_ISOTP_TX_SEND_SF,
    SYN_ISOTP_TX_SEND_FF,
    SYN_ISOTP_TX_WAIT_FC,
    SYN_ISOTP_TX_SEND_CF,
} SYN_ISOTP_TxState;

typedef enum {
    SYN_ISOTP_RX_IDLE = 0,
    SYN_ISOTP_RX_WAIT_CF,
    SYN_ISOTP_RX_COMPLETE,
} SYN_ISOTP_RxState;

typedef struct {
    /* Addressing & Mode */
    uint32_t            rx_id;          
    uint32_t            tx_id;          
#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
    bool                is_fd;          
#endif

    /* Tx Channel */
    uint8_t            *tx_buf;         
    size_t              tx_buf_size;    
    size_t              tx_len;         
    size_t              tx_offset;      
    uint8_t             tx_seq;         
    SYN_ISOTP_TxState   tx_state;       
    uint8_t             tx_bs;          
    uint8_t             tx_bs_count;    
    uint8_t             tx_st_min;      
    uint32_t            tx_st_timer_us; 
    uint32_t            n_bs_timeout_us;
    uint32_t            tx_timeout_timer_us; 
    /* Rx Channel */
    uint8_t            *rx_buf;         
    size_t              rx_buf_size;    
    size_t              rx_len;         
    size_t              rx_expected;    
    uint8_t             rx_seq;         
    SYN_ISOTP_RxState   rx_state;       
    bool                rx_fc_pending;  
    uint8_t             rx_fc_status;   
    uint32_t            n_cr_timeout_us;
    uint32_t            rx_timeout_timer_us; 
} SYN_ISOTP_Link;

void syn_isotp_set_timeouts(SYN_ISOTP_Link *link, uint32_t n_bs_ms, uint32_t n_cr_ms);

void syn_isotp_init(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                    uint8_t *rx_buf, size_t rx_buf_size,
                    uint8_t *tx_buf, size_t tx_buf_size);

#if defined(SYN_USE_CAN_FD) && SYN_USE_CAN_FD
void syn_isotp_init_fd(SYN_ISOTP_Link *link, uint32_t rx_id, uint32_t tx_id,
                       uint8_t *rx_buf, size_t rx_buf_size,
                       uint8_t *tx_buf, size_t tx_buf_size, bool is_fd);
#endif

SYN_Status syn_isotp_send(SYN_ISOTP_Link *link, const uint8_t *payload, size_t payload_len);

bool syn_isotp_get_tx_frame(SYN_ISOTP_Link *link, SYN_CAN_Frame *frame);

void syn_isotp_process_rx_frame(SYN_ISOTP_Link *link, const SYN_CAN_Frame *frame);

ssize_t syn_isotp_receive(SYN_ISOTP_Link *link, uint8_t *out_buf, size_t max_len);

void syn_isotp_step(SYN_ISOTP_Link *link, uint32_t dt_ms);

void syn_isotp_step_us(SYN_ISOTP_Link *link, uint32_t dt_us);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ISOTP_H */
```


