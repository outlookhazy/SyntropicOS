

# File syn\_canopen\_mgr.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen\_mgr.h**](syn__canopen__mgr_8h.md)

[Go to the documentation of this file](syn__canopen__mgr_8h.md)


```C++

#ifndef SYN_CANOPEN_MGR_H
#define SYN_CANOPEN_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_canopen.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_SDO_CLIENT_STATE_IDLE = 0,
    SYN_SDO_CLIENT_STATE_WAIT_READ,
    SYN_SDO_CLIENT_STATE_WAIT_WRITE,
    SYN_SDO_CLIENT_STATE_SUCCESS,
    SYN_SDO_CLIENT_STATE_ERROR
} SYN_SDOClientState;

typedef struct {
    uint8_t            target_node;   
    uint16_t           index;         
    uint8_t            subindex;      
    SYN_SDOClientState state;         
    uint32_t           abort_code;    
    uint8_t            data[8];       
    size_t             data_len;      
    uint16_t           timeout_ms;    
} SYN_CANOpenSDOClient;

typedef struct {
    uint8_t  node_id;       
    uint8_t  nmt_state;     
    uint16_t timer_ms;      
    bool     online;        
} SYN_CANOpenNodeMonitor;

typedef struct {
    SYN_CANOpenSDOClient  sdo_client;     
    SYN_CANOpenNodeMonitor nodes[128];    
} SYN_CANOpenManager;

void syn_canopen_mgr_init(SYN_CANOpenManager *mgr);

void syn_canopen_mgr_build_nmt(SYN_CAN_Frame *frame, uint8_t target_node, uint8_t nmt_cmd);

SYN_Status syn_canopen_mgr_sdo_read_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                          uint8_t node_id, uint16_t index, uint8_t subindex);

SYN_Status syn_canopen_mgr_sdo_write_init(SYN_CANOpenManager *mgr, SYN_CAN_Frame *frame,
                                           uint8_t node_id, uint16_t index, uint8_t subindex,
                                           const void *data, size_t len);

void syn_canopen_mgr_process_frame(SYN_CANOpenManager *mgr, const SYN_CAN_Frame *frame);

void syn_canopen_mgr_step(SYN_CANOpenManager *mgr, uint16_t delta_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CANOPEN_MGR_H */
```


