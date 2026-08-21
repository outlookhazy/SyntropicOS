

# File syn\_iolink.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_iolink.h**](syn__iolink_8h.md)

[Go to the documentation of this file](syn__iolink_8h.md)


```C++

#ifndef SYN_IOLINK_H
#define SYN_IOLINK_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_IOLINK) || SYN_USE_IOLINK

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Parameter Page Addresses ───────────────────────────────── */

#define SYN_IOLINK_PAGE_MASTER_CYCLE_TIME 0x00U 
#define SYN_IOLINK_PAGE_MIN_CYCLE_TIME 0x01U    
#define SYN_IOLINK_PAGE_MSEQ_CAPABILITY 0x02U   
#define SYN_IOLINK_PAGE_REVISION_ID 0x03U       
#define SYN_IOLINK_PAGE_PD_IN_LEN 0x04U         
#define SYN_IOLINK_PAGE_PD_OUT_LEN 0x05U        
#define SYN_IOLINK_PAGE_VENDOR_ID_1 0x06U       
#define SYN_IOLINK_PAGE_VENDOR_ID_2 0x07U       
#define SYN_IOLINK_PAGE_DEVICE_ID_1 0x08U       
#define SYN_IOLINK_PAGE_DEVICE_ID_2 0x09U       
#define SYN_IOLINK_PAGE_DEVICE_ID_3 0x0AU       
#define SYN_IOLINK_REV_1_0 0x10U 
#define SYN_IOLINK_REV_1_1 0x11U 
#define SYN_IOLINK_MAX_PD_LEN 32U   
#define SYN_IOLINK_MAX_ISDU_LEN 64U 
/* ── Type Definitions ────────────────────────────────────────────────────── */

typedef enum {
    SYN_IOLINK_BAUD_COM1 = 0, 
    SYN_IOLINK_BAUD_COM2 = 1, 
    SYN_IOLINK_BAUD_COM3 = 2  
} SYN_IOLink_Baud;

typedef enum {
    SYN_IOLINK_PORT_INACTIVE = 0, 
    SYN_IOLINK_PORT_SIO,          
    SYN_IOLINK_PORT_STARTUP,      
    SYN_IOLINK_PORT_OPERATE,      
    SYN_IOLINK_PORT_FAULT         
} SYN_IOLink_PortState;

typedef enum {
    SYN_IOLINK_TYPE_0 = 0,   
    SYN_IOLINK_TYPE_1_1 = 1, 
    SYN_IOLINK_TYPE_1_2 = 2, 
    SYN_IOLINK_TYPE_2_1 = 3, 
    SYN_IOLINK_TYPE_2_2 = 4, 
    SYN_IOLINK_TYPE_2_V = 5  
} SYN_IOLink_MSeqType;

typedef struct {
    uint8_t master_cycle_time; 
    uint8_t min_cycle_time;    
    uint8_t mseq_capability;   
    uint8_t revision_id;       
    uint8_t pd_in_len;         
    uint8_t pd_out_len;        
    uint16_t vendor_id;        
    uint32_t device_id;        
} SYN_IOLink_DirectParams;

typedef SYN_Status (*SYN_IOLink_ISDU_ReadCb)(uint16_t index, uint8_t subindex, uint8_t *out_data,
                                             size_t max_len, size_t *out_len, void *user_data);

typedef SYN_Status (*SYN_IOLink_ISDU_WriteCb)(uint16_t index, uint8_t subindex, const uint8_t *data,
                                              size_t len, void *user_data);

typedef struct {
    SYN_IOLink_Baud target_baud; 
    SYN_Transport *transport;    
    uint32_t cycle_time_ms;      
    void *user_data;             
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
} SYN_IOLink_MasterConfig;

typedef struct {
    SYN_IOLink_MasterConfig cfg; 
    SYN_IOLink_PortState state;  
    SYN_PT pt;                   
    SYN_IOLink_DirectParams dev_params; 
    SYN_IOLink_MSeqType mseq_type;      
    uint32_t last_cycle_ms;             
    uint8_t current_pd_in[SYN_IOLINK_MAX_PD_LEN];  
    size_t current_pd_in_len;                      
    uint8_t current_pd_out[SYN_IOLINK_MAX_PD_LEN]; 
    size_t current_pd_out_len;                     
} SYN_IOLink_Master;

typedef struct {
    SYN_IOLink_DirectParams params;   
    SYN_IOLink_ISDU_ReadCb on_read;   
    SYN_IOLink_ISDU_WriteCb on_write; 
    void *user_data;                  
} SYN_IOLink_DeviceConfig;

typedef struct {
    SYN_IOLink_DeviceConfig cfg; 
    SYN_IOLink_PortState state;  
    uint8_t pd_in[SYN_IOLINK_MAX_PD_LEN];  
    uint8_t pd_out[SYN_IOLINK_MAX_PD_LEN]; 
} SYN_IOLink_Device;

/* ── CRC & Framing Functions ─────────────────────────────────────────────── */

uint8_t syn_iolink_calc_checksum(const uint8_t *data, size_t len);

/* ── Master Public API ───────────────────────────────────────────────────── */

SYN_Status syn_iolink_master_init(SYN_IOLink_Master *master, const SYN_IOLink_MasterConfig *cfg);

SYN_Status syn_iolink_master_start(SYN_IOLink_Master *master);

SYN_Status syn_iolink_master_exchange_pd(SYN_IOLink_Master *master, const uint8_t *pd_out,
                                         size_t out_len, uint8_t *pd_in, size_t *in_len);

SYN_Status syn_iolink_master_read_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                       uint8_t *data, size_t max_len, size_t *out_len);

SYN_Status syn_iolink_master_write_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                        const uint8_t *data, size_t len);

SYN_Status syn_iolink_master_step(SYN_IOLink_Master *master, uint32_t now_ms);

SYN_PT_Status syn_iolink_master_pt(SYN_PT *pt, SYN_Task *task);

/* ── Device Public API ───────────────────────────────────────────────────── */

SYN_Status syn_iolink_device_init(SYN_IOLink_Device *dev, const SYN_IOLink_DeviceConfig *cfg);

SYN_Status syn_iolink_device_set_pd_in(SYN_IOLink_Device *dev, const uint8_t *data, size_t len);

SYN_Status syn_iolink_device_get_pd_out(const SYN_IOLink_Device *dev, uint8_t *out_buf,
                                        size_t max_len, size_t *out_len);

SYN_Status syn_iolink_device_process_frame(SYN_IOLink_Device *dev, const uint8_t *rx, size_t rx_len,
                                           uint8_t *tx, size_t max_tx, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_IOLINK */

#endif /* SYN_IOLINK_H */
```


