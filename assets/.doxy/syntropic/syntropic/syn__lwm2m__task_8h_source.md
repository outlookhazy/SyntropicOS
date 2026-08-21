

# File syn\_lwm2m\_task.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m\_task.h**](syn__lwm2m__task_8h.md)

[Go to the documentation of this file](syn__lwm2m__task_8h.md)


```C++

#ifndef SYN_LWM2M_TASK_H
#define SYN_LWM2M_TASK_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_coap.h"
#include "syntropic/proto/syn_lwm2m.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#if !defined(SYN_USE_LWM2M_TASK) || SYN_USE_LWM2M_TASK

#ifdef __cplusplus
extern "C" {
#endif

/* ── Forward Declarations & Callbacks ────────────────────────────────────── */

struct SYN_LwM2M_Task;

typedef void (*SYN_LwM2M_RebootCb)(void *user_data);

typedef void (*SYN_LwM2M_FactoryResetCb)(void *user_data);

typedef void (*SYN_LwM2M_FwUpdateCb)(const char *package_uri, void *user_data);

typedef void (*SYN_LwM2M_StateChangeCb)(SYN_LwM2M_ClientState old_state,
                                        SYN_LwM2M_ClientState new_state, void *user_data);

/* ── Configuration Structure ─────────────────────────────────────────────── */

typedef struct {
    SYN_LwM2M_Client *client;          
    SYN_Transport *transport;          
    SYN_CoapsClient *coaps_client;     
    uint32_t retry_backoff_ms;         
    SYN_LwM2M_RebootCb on_reboot;      
    SYN_LwM2M_FactoryResetCb on_reset; 
    SYN_LwM2M_FwUpdateCb on_fw_update; 
    SYN_LwM2M_StateChangeCb on_state;  
    void *user_data;                   
    /* Scratch frame buffers */
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
} SYN_LwM2M_TaskConfig;

/* ── Task Context ────────────────────────────────────────────────────────── */

typedef struct SYN_LwM2M_Task {
    SYN_LwM2M_TaskConfig cfg; 
    SYN_PT pt;                
    uint16_t next_msg_id;     
    uint32_t last_action_ms;  
    uint32_t retry_count;     
    bool force_update;        
    bool force_deregister;    
    bool resource_changed;    
    uint16_t changed_obj_id;  
    uint16_t changed_inst_id; 
    int32_t changed_res_id;   
} SYN_LwM2M_Task;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_lwm2m_task_init(SYN_LwM2M_Task *task, const SYN_LwM2M_TaskConfig *cfg);

SYN_Status syn_lwm2m_task_step(SYN_LwM2M_Task *task, uint32_t now_ms);

SYN_PT_Status syn_lwm2m_task_pt(SYN_PT *pt, SYN_Task *task);

SYN_Status syn_lwm2m_task_trigger_update(SYN_LwM2M_Task *task);

SYN_Status syn_lwm2m_task_deregister(SYN_LwM2M_Task *task);

SYN_Status syn_lwm2m_task_notify_changed(SYN_LwM2M_Task *task, uint16_t obj_id, uint16_t inst_id,
                                         int32_t res_id);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LWM2M_TASK */

#endif /* SYN_LWM2M_TASK_H */
```


