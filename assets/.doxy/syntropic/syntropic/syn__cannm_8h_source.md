

# File syn\_cannm.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cannm.h**](syn__cannm_8h.md)

[Go to the documentation of this file](syn__cannm_8h.md)


```C++

#ifndef SYN_CANNM_H
#define SYN_CANNM_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CANNM) || SYN_USE_CANNM

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_CANNM_CBV_REPEAT_MSG_REQ 0x01U    
#define SYN_CANNM_CBV_PNI_REQ 0x40U           
#define SYN_CANNM_CBV_ACTIVE_WAKEUP_REQ 0x10U 
typedef enum {
    SYN_CANNM_STATE_BUS_SLEEP = 0, 
    SYN_CANNM_STATE_PRE_BUS_SLEEP, 
    SYN_CANNM_STATE_REPEAT_MSG,    
    SYN_CANNM_STATE_NORMAL_OP,     
    SYN_CANNM_STATE_READY_SLEEP    
} SYN_CanNM_State;

typedef struct {
    uint8_t node_id;             
    uint32_t can_id_base;        
    uint32_t can_id_mask;        
    uint32_t msg_cycle_ms;       
    uint32_t nm_timeout_ms;      
    uint32_t wait_bus_sleep_ms;  
    uint32_t repeat_msg_time_ms; 
} SYN_CanNM_Config;

typedef struct {
    SYN_CanNM_Config config;    
    SYN_CanNM_State state;      
    SYN_CanNM_State prev_state; 
    bool node_comm_req;  
    bool repeat_msg_req; 
    uint32_t msg_cycle_timer;      
    uint32_t timeout_timer;        
    uint32_t wait_bus_sleep_timer; 
    uint32_t repeat_msg_timer;     
    uint8_t user_data[6];      
    uint8_t rx_user_data[6];   
    uint8_t rx_source_node_id; 
    uint8_t rx_cbv;            
} SYN_CanNM_Session;

void syn_cannm_init(SYN_CanNM_Session *session, const SYN_CanNM_Config *cfg);

void syn_cannm_request_network(SYN_CanNM_Session *session);

void syn_cannm_release_network(SYN_CanNM_Session *session);

void syn_cannm_request_repeat_msg(SYN_CanNM_Session *session);

void syn_cannm_set_user_data(SYN_CanNM_Session *session, const uint8_t *data, size_t len);

bool syn_cannm_process_rx_frame(SYN_CanNM_Session *session, const SYN_CAN_Frame *frame);

bool syn_cannm_step(SYN_CanNM_Session *session, uint32_t delta_ms, SYN_CAN_Frame *tx_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_CANNM */
#endif /* SYN_CANNM_H */
```


