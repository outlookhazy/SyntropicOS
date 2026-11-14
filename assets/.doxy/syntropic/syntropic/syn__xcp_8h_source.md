

# File syn\_xcp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xcp.h**](syn__xcp_8h.md)

[Go to the documentation of this file](syn__xcp_8h.md)


```C++


#ifndef SYN_XCP_H
#define SYN_XCP_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_XCP_MAX_DAQ_LISTS
#define SYN_XCP_MAX_DAQ_LISTS 4U 
#endif

#ifndef SYN_XCP_MAX_ODT_PER_DAQ
#define SYN_XCP_MAX_ODT_PER_DAQ 8U 
#endif

#ifndef SYN_XCP_MAX_ENTRIES_PER_ODT
#define SYN_XCP_MAX_ENTRIES_PER_ODT 7U 
#endif

#ifndef SYN_XCP_MAX_MEM_REGIONS
#define SYN_XCP_MAX_MEM_REGIONS 4U 
#endif

#define SYN_XCP_CMD_CONNECT 0xFFU             
#define SYN_XCP_CMD_DISCONNECT 0xFEU          
#define SYN_XCP_CMD_GET_STATUS 0xFDU          
#define SYN_XCP_CMD_SYNCH 0xFCU               
#define SYN_XCP_CMD_GET_COMM_MODE_INFO 0xFBU  
#define SYN_XCP_CMD_GET_ID 0xFAU              
#define SYN_XCP_CMD_SET_REQUEST 0xF9U         
#define SYN_XCP_CMD_GET_SEED 0xF8U            
#define SYN_XCP_CMD_UNLOCK 0xF7U              
#define SYN_XCP_CMD_SET_MTA 0xF6U             
#define SYN_XCP_CMD_UPLOAD 0xF5U              
#define SYN_XCP_CMD_SHORT_UPLOAD 0xF4U        
#define SYN_XCP_CMD_DOWNLOAD 0xF0U            
#define SYN_XCP_CMD_DOWNLOAD_MAX 0xEEU        
#define SYN_XCP_CMD_SET_DAQ_PTR 0xE2U         
#define SYN_XCP_CMD_WRITE_DAQ 0xE1U           
#define SYN_XCP_CMD_SET_DAQ_LIST_MODE 0xE0U   
#define SYN_XCP_CMD_START_STOP_DAQ_LIST 0xDEU 
#define SYN_XCP_CMD_START_STOP_SYNCH 0xDDU    

#define SYN_XCP_PID_RES 0xFFU  
#define SYN_XCP_PID_ERR 0xFEU  
#define SYN_XCP_PID_EV 0xFDU   
#define SYN_XCP_PID_SERV 0xFCU 

#define SYN_XCP_ERR_SUCCESS 0x00U         
#define SYN_XCP_ERR_CMD_SYNTAX 0x20U      
#define SYN_XCP_ERR_OUT_OF_RANGE 0x22U    
#define SYN_XCP_ERR_WRITE_PROTECTED 0x23U 
#define SYN_XCP_ERR_ACCESS_DENIED 0x24U   
#define SYN_XCP_ERR_ACCESS_LOCKED 0x25U   
#define SYN_XCP_ERR_NOT_CONNECTED 0x28U   
#define SYN_XCP_ERR_CMD_UNKNOWN 0x30U     

#define SYN_XCP_RESOURCE_CAL_PAG 0x01U 
#define SYN_XCP_RESOURCE_DAQ 0x04U     
#define SYN_XCP_RESOURCE_STIM 0x08U    
#define SYN_XCP_RESOURCE_PGM 0x10U     

typedef struct {
    uintptr_t address; 
    uint8_t size;      
    bool host_valid;   
} SYN_XCP_ODTEntry;

typedef struct {
    uint32_t base_addr; 
    uint32_t size;      
    uint8_t *host_ptr;  
    bool writable;      
} SYN_XCP_MemRegion;

typedef uint8_t (*SYN_XCP_SeedFn)(uint8_t resource, uint8_t seed_out[6], void *ctx);

typedef bool (*SYN_XCP_UnlockFn)(uint8_t resource, const uint8_t *key, uint8_t key_len, void *ctx);

typedef struct {
    SYN_XCP_ODTEntry entries[SYN_XCP_MAX_ENTRIES_PER_ODT]; 
    uint8_t entry_count;                                   
} SYN_XCP_ODT;

typedef struct {
    SYN_XCP_ODT odts[SYN_XCP_MAX_ODT_PER_DAQ]; 
    uint8_t odt_count;                         
    uint8_t current_odt_idx;                   
    uint8_t current_entry_idx;                 
    bool running;                              
    bool pending_tx;                           
    uint8_t mode;                              
    uint8_t event_channel;                     
    uint16_t prescaler;                        
    uint16_t cycle_counter;                    
} SYN_XCP_DAQList;

typedef struct {
    uint16_t station_id;             
    bool connected;                  
    uint8_t unlocked_resources;      
    uint8_t pending_unlock_resource; 
    uint8_t mta_ext;                 
    uintptr_t mta_addr;              
    bool mta_host_valid;             
    uint8_t rr_daq_idx;              
    bool daq_tick_drained;           
    uint8_t daq_tick_ch;             
    uint8_t current_daq_ptr_list;    
    uint8_t current_daq_ptr_odt;     
    uint8_t current_daq_ptr_idx;     
    SYN_XCP_SeedFn seed_cb;          
    void *seed_ctx;                  
    SYN_XCP_UnlockFn unlock_cb;      
    void *unlock_ctx;                
    SYN_XCP_MemRegion mem_regions[SYN_XCP_MAX_MEM_REGIONS]; 
    uint8_t region_count;                                   
    SYN_XCP_DAQList daq_lists[SYN_XCP_MAX_DAQ_LISTS];       
} SYN_XCP_Slave;

bool syn_xcp_init(SYN_XCP_Slave *slave, uint16_t station_id);

bool syn_xcp_set_mta(SYN_XCP_Slave *slave, uint8_t ext, uintptr_t addr);

bool syn_xcp_register_mem_region(SYN_XCP_Slave *slave, uint32_t base_addr, void *host_ptr,
                                 uint32_t size, bool writable);

bool syn_xcp_process_cto(SYN_XCP_Slave *slave, const uint8_t cto[8], uint8_t dto_out[8]);

void syn_xcp_daq_event(SYN_XCP_Slave *slave, uint8_t event_channel);

bool syn_xcp_service_daq(SYN_XCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_XCP_H */
```


