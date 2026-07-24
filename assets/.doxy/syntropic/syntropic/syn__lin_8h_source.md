

# File syn\_lin.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lin.h**](syn__lin_8h.md)

[Go to the documentation of this file](syn__lin_8h.md)


```C++

#ifndef SYN_LIN_H
#define SYN_LIN_H

#include "../common/syn_defs.h"

#if defined(SYN_USE_LIN) && SYN_USE_LIN

#ifdef __cplusplus
extern "C" {
#endif

/* ── LIN Frame Identifiers & Limits ──────────────────────────────────────── */

#define SYN_LIN_ID_MAX             0x3F  
#define SYN_LIN_DATA_MAX           8     
#define SYN_LIN_ID_MASTER_REQ      0x3C  
#define SYN_LIN_ID_SLAVE_RESP      0x3D  
#define SYN_LIN_ID_EXTENDED        0x3E  
#define SYN_LIN_ID_RESERVED        0x3F  
#define SYN_LIN_SYNC_BYTE          0x55  
/* ── LIN Checksum Modes ─────────────────────────────────────────────────── */

typedef enum {
    SYN_LIN_CHECKSUM_CLASSIC = 0,  
    SYN_LIN_CHECKSUM_ENHANCED = 1  
} SYN_LIN_ChecksumMode;

/* ── LIN Frame Data Structure ────────────────────────────────────────────── */

typedef struct {
    uint8_t id;                       
    uint8_t pid;                      
    uint8_t len;                      
    uint8_t data[SYN_LIN_DATA_MAX];   
    uint8_t checksum;                 
    bool    valid_pid;                
    bool    valid_checksum;           
} SYN_LIN_Frame;

/* ── LIN Master Schedule Table Slot ──────────────────────────────────────── */

typedef enum {
    SYN_LIN_SLOT_PUBLISH = 0,   
    SYN_LIN_SLOT_SUBSCRIBE = 1  
} SYN_LIN_SlotDirection;

typedef struct {
    uint8_t              id;          
    uint8_t              len;         
    SYN_LIN_ChecksumMode checksum_mode; 
    SYN_LIN_SlotDirection dir;        
    uint8_t              data[SYN_LIN_DATA_MAX]; 
    uint32_t             delay_ms;    
} SYN_LIN_ScheduleSlot;

/* ── LIN Master Handle ──────────────────────────────────────────────────── */

typedef struct {
    const SYN_LIN_ScheduleSlot *slots;       
    size_t                      slot_count;  
    size_t                      current_slot;
    uint32_t                    slot_timer;  
    bool                        running;     
} SYN_LIN_Master;

/* ── LIN Slave Published/Subscribed Frame Filter ─────────────────────────── */

typedef struct {
    uint8_t              id;            
    uint8_t              len;           
    SYN_LIN_ChecksumMode checksum_mode; 
    SYN_LIN_SlotDirection dir;          
    uint8_t              data[SYN_LIN_DATA_MAX]; 
    bool                 active;        
} SYN_LIN_SlaveFrame;

/* ── LIN Slave Handle ───────────────────────────────────────────────────── */

#ifndef SYN_LIN_SLAVE_MAX_FRAMES
  #define SYN_LIN_SLAVE_MAX_FRAMES 8  
#endif

typedef enum {
    SYN_LIN_STATE_IDLE = 0,
    SYN_LIN_STATE_SYNC,
    SYN_LIN_STATE_PID,
    SYN_LIN_STATE_DATA,
    SYN_LIN_STATE_CHECKSUM
} SYN_LIN_State;

typedef struct {
    uint8_t            nad;                                
    SYN_LIN_SlaveFrame frames[SYN_LIN_SLAVE_MAX_FRAMES];   
    size_t             frame_count;                        
    SYN_LIN_State      state;                              
    SYN_LIN_Frame      rx_frame;                           
    uint8_t            rx_idx;                             
    uint8_t            expected_len;                       
    SYN_LIN_ChecksumMode expected_checksum_mode;          
} SYN_LIN_Slave;

/* ── API Functions ──────────────────────────────────────────────────────── */

uint8_t syn_lin_calc_pid(uint8_t id);

bool syn_lin_verify_pid(uint8_t pid);

uint8_t syn_lin_calc_checksum(uint8_t pid, const uint8_t *data, uint8_t len, SYN_LIN_ChecksumMode mode);

SYN_Status syn_lin_master_init(SYN_LIN_Master *master, const SYN_LIN_ScheduleSlot *slots, size_t count);

bool syn_lin_master_step(SYN_LIN_Master *master, uint32_t dt_ms, const SYN_LIN_ScheduleSlot **active_slot);

SYN_Status syn_lin_slave_init(SYN_LIN_Slave *slave, uint8_t nad);

SYN_Status syn_lin_slave_add_frame(SYN_LIN_Slave *slave, uint8_t id, uint8_t len, SYN_LIN_SlotDirection dir, SYN_LIN_ChecksumMode mode);

SYN_Status syn_lin_slave_set_publish_data(SYN_LIN_Slave *slave, uint8_t id, const uint8_t *data, uint8_t len);

bool syn_lin_slave_process_byte(SYN_LIN_Slave *slave, uint8_t byte, SYN_LIN_Frame *out_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LIN */

#endif /* SYN_LIN_H */
```


