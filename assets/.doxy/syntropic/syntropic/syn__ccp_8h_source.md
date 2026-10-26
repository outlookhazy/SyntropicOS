

# File syn\_ccp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ccp.h**](syn__ccp_8h.md)

[Go to the documentation of this file](syn__ccp_8h.md)


```C++

#ifndef SYN_CCP_H
#define SYN_CCP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_CCP_MAX_DAQ_LISTS
#define SYN_CCP_MAX_DAQ_LISTS 4U
#endif

#ifndef SYN_CCP_MAX_ODT_PER_DAQ
#define SYN_CCP_MAX_ODT_PER_DAQ 8U
#endif

#ifndef SYN_CCP_MAX_ENTRIES_PER_ODT
#define SYN_CCP_MAX_ENTRIES_PER_ODT 7U
#endif

#define SYN_CCP_CMD_CONNECT 0x01U             
#define SYN_CCP_CMD_EXCHANGE_ID 0x02U         
#define SYN_CCP_CMD_DNLOAD 0x03U              
#define SYN_CCP_CMD_UPLOAD 0x04U              
#define SYN_CCP_CMD_START_STOP_ALL 0x06U      
#define SYN_CCP_CMD_SET_MTA 0x08U             
#define SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE 0x09U 
#define SYN_CCP_CMD_SHORT_UP 0x0FU            
#define SYN_CCP_CMD_BUILD_PKT 0x10U           
#define SYN_CCP_CMD_START_STOP 0x11U          
#define SYN_CCP_CMD_GET_SEED 0x12U            
#define SYN_CCP_CMD_UNLOCK 0x13U              
#define SYN_CCP_CMD_SET_CAL_PAGE 0x14U        
#define SYN_CCP_CMD_SET_DAQ_SIZE 0x15U        
#define SYN_CCP_CMD_DISCONNECT 0x17U          
#define SYN_CCP_CMD_CLEAR_MEMORY 0x18U        
#define SYN_CCP_CMD_GET_DAQ_SIZE 0x1AU        
#define SYN_CCP_CMD_GET_CCP_VERSION 0x1BU     
#define SYN_CCP_CMD_PROGRAM 0x22U             
#define SYN_CCP_CMD_DNLOAD_6 0x23U            
#define SYN_CCP_PID_CRM 0xFFU   
#define SYN_CCP_PID_EVENT 0xFEU 
#define SYN_CCP_ERR_SUCCESS 0x00U            
#define SYN_CCP_ERR_CMD_UNKNOWN 0x01U        
#define SYN_CCP_ERR_CMD_SYNTAX 0x02U         
#define SYN_CCP_ERR_PARAM_OUT_OF_RANGE 0x10U 
#define SYN_CCP_ERR_ACCESS_DENIED 0x11U      
#define SYN_CCP_ERR_NOT_CONNECTED 0x12U      
#define SYN_CCP_ERR_RESOURCE_LOCKED 0x18U    
#define SYN_CCP_RESOURCE_CAL 0x01U 
#define SYN_CCP_RESOURCE_DAQ 0x02U 
#define SYN_CCP_RESOURCE_PGM 0x40U 
typedef struct {
    uintptr_t address; 
    uint8_t size;      
} SYN_CCP_ODTEntry;

typedef struct {
    SYN_CCP_ODTEntry entries[SYN_CCP_MAX_ENTRIES_PER_ODT]; 
    uint8_t entry_count;                                   
} SYN_CCP_ODT;

typedef struct {
    SYN_CCP_ODT odts[SYN_CCP_MAX_ODT_PER_DAQ]; 
    uint8_t odt_count;                         
    uint8_t current_odt_idx;                   
    uint8_t current_entry_idx;                 
    bool running;                              
    uint8_t event_channel;                     
    uint16_t prescaler;                        
    uint16_t cycle_counter;                    
} SYN_CCP_DAQList;

typedef uint32_t (*SYN_CCP_SeedFn)(uint8_t resource, void *ctx);

typedef bool (*SYN_CCP_UnlockFn)(uint8_t resource, uint32_t key, void *ctx);

typedef bool (*SYN_CCP_EraseFn)(uint32_t addr, uint32_t size, void *ctx);

typedef struct {
    uint16_t station_addr;                            
    bool connected;                                   
    uint8_t unlocked_resources;                       
    uintptr_t mta0_addr;                              
    uint8_t mta0_ext;                                 
    uintptr_t mta1_addr;                              
    uint8_t mta1_ext;                                 
    uint8_t active_cal_page;                          
    SYN_CCP_DAQList daq_lists[SYN_CCP_MAX_DAQ_LISTS]; 
    uint8_t selected_daq;                             
    uint8_t selected_odt;                             
    SYN_CCP_SeedFn seed_cb;                           
    void *seed_ctx;                                   
    SYN_CCP_UnlockFn unlock_cb; 
    void *unlock_ctx;           
    SYN_CCP_EraseFn erase_cb;   
    void *erase_ctx;            
} SYN_CCP_Slave;

void syn_ccp_init(SYN_CCP_Slave *slave, uint16_t station_addr);

bool syn_ccp_process_cro(SYN_CCP_Slave *slave, const uint8_t cro[8], uint8_t dto_out[8]);

void syn_ccp_set_mta(SYN_CCP_Slave *slave, uint8_t mta_num, uint8_t ext, uintptr_t addr);

bool syn_ccp_service_daq(SYN_CCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CCP_H */
```


