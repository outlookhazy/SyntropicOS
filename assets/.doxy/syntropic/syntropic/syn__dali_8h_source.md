

# File syn\_dali.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.h**](syn__dali_8h.md)

[Go to the documentation of this file](syn__dali_8h.md)


```C++

#ifndef SYN_DALI_H
#define SYN_DALI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DALI Constants ─────────────────────────────────────────────────────── */

#define SYN_DALI_SHORT_ADDR_UNASSIGNED   0xFFU   
#define SYN_DALI_MASK_LEVEL              255U    
typedef enum {
    SYN_DALI_ADDR_SHORT = 0,    
    SYN_DALI_ADDR_GROUP,        
    SYN_DALI_ADDR_BROADCAST,    
    SYN_DALI_ADDR_SPECIAL,      
} SYN_DALI_AddressType;

typedef struct {
    SYN_DALI_AddressType  addr_type;  
    uint8_t               address;    
    bool                  is_direct;  
    uint8_t               data_cmd;   
    uint16_t              raw;        
} SYN_DALI_ForwardFrame;

typedef struct {
    uint8_t               data;       
} SYN_DALI_BackwardFrame;

typedef struct {
    uint8_t               short_address;        
    uint16_t              group_mask;           
    uint8_t               min_level;            
    uint8_t               max_level;            
    uint8_t               power_on_level;       
    uint8_t               system_failure_level; 
    uint8_t               fade_rate;            
    uint8_t               fade_time;            
} SYN_DALI_SlaveConfig;

typedef struct {
    SYN_DALI_SlaveConfig  cfg;
    uint8_t               actual_level;         
    uint8_t               scenes[16];           
    uint32_t              random_address;       
    uint32_t              search_address;       
    bool                  initialise_active;    
    bool                  lamp_failure;         
    bool                  control_gear_failure; 
    bool                  lamp_on;              
    uint8_t               dtr0;                 
    uint8_t               dtr1;                 
    uint8_t               dtr2;                 
    bool                  write_mem_enabled;    
} SYN_DALI_SlaveState;

/* ── Standard DALI Command Definitions (IEC 62386-102) ─────────────────── */

#define SYN_DALI_CMD_OFF                         0x00U
#define SYN_DALI_CMD_UP                          0x01U
#define SYN_DALI_CMD_DOWN                        0x02U
#define SYN_DALI_CMD_STEP_UP                     0x03U
#define SYN_DALI_CMD_STEP_DOWN                   0x04U
#define SYN_DALI_CMD_RECALL_MAX                  0x05U
#define SYN_DALI_CMD_RECALL_MIN                  0x06U
#define SYN_DALI_CMD_STEP_DOWN_AND_OFF           0x07U
#define SYN_DALI_CMD_ON_AND_STEP_UP              0x08U
#define SYN_DALI_CMD_GO_TO_SCENE_BASE            0x10U /* 0x10..0x1F */

#define SYN_DALI_CMD_RESET                       0x20U
#define SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR   0x21U
#define SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL      0x2AU
#define SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL      0x2BU
#define SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL 0x2CU
#define SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL 0x2DU
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME      0x2EU
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE      0x2FU

#define SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE     0x40U /* 0x40..0x4F */
#define SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE      0x50U /* 0x50..0x5F */
#define SYN_DALI_CMD_ADD_TO_GROUP_BASE           0x60U /* 0x60..0x6F */
#define SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE      0x70U /* 0x70..0x7F */
#define SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR     0x80U
#define SYN_DALI_CMD_ENABLE_WRITE_MEMORY         0x81U

#define SYN_DALI_CMD_QUERY_STATUS                0x90U
#define SYN_DALI_CMD_QUERY_CONTROL_GEAR          0x91U
#define SYN_DALI_CMD_QUERY_LAMP_FAILURE          0x92U
#define SYN_DALI_CMD_QUERY_LAMP_POWER_ON         0x93U
#define SYN_DALI_CMD_QUERY_CONTENT_DTR           0x98U
#define SYN_DALI_CMD_QUERY_DEVICE_TYPE           0x99U
#define SYN_DALI_CMD_QUERY_PHYSICAL_MIN_LEVEL    0x9AU
#define SYN_DALI_CMD_QUERY_CONTENT_DTR1          0x9CU
#define SYN_DALI_CMD_QUERY_CONTENT_DTR2          0x9DU
#define SYN_DALI_CMD_QUERY_ACTUAL_LEVEL          0xA0U
#define SYN_DALI_CMD_QUERY_MAX_LEVEL             0xA1U
#define SYN_DALI_CMD_QUERY_MIN_LEVEL             0xA2U
#define SYN_DALI_CMD_QUERY_POWER_ON_LEVEL        0xA3U
#define SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL        0xA4U

#define SYN_DALI_CMD_QUERY_GROUPS_0_7            0xC0U
#define SYN_DALI_CMD_QUERY_GROUPS_8_15           0xC1U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_H         0xC2U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_M         0xC3U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_L         0xC4U

/* Special / Configuration Commands */
#define SYN_DALI_SPEC_TERMINATE                  0x00U
#define SYN_DALI_SPEC_DTR0                       0xA3U
#define SYN_DALI_SPEC_INITIALISE                 0x01U
#define SYN_DALI_SPEC_RANDOMISE                  0x02U
#define SYN_DALI_SPEC_COMPARE                    0x03U
#define SYN_DALI_SPEC_WITHDRAW                   0x04U
#define SYN_DALI_SPEC_SEARCHADDRH                0x05U
#define SYN_DALI_SPEC_SEARCHADDRM                0x06U
#define SYN_DALI_SPEC_SEARCHADDRL                0x07U
#define SYN_DALI_SPEC_PROGRAM_SHORT_ADDR         0x08U
#define SYN_DALI_SPEC_VERIFY_SHORT_ADDR          0x09U
#define SYN_DALI_SPEC_QUERY_SHORT_ADDR           0x0AU
#define SYN_DALI_SPEC_DTR1                       0xC3U
#define SYN_DALI_SPEC_DTR2                       0xC5U

/* ── API Functions ──────────────────────────────────────────────────────── */

uint16_t syn_dali_encode_forward(uint8_t addr, uint8_t data_cmd);

bool syn_dali_decode_forward(uint16_t raw_16, SYN_DALI_ForwardFrame *frame);

uint8_t syn_dali_encode_backward(uint8_t data);

bool syn_dali_decode_backward(uint8_t raw_8, SYN_DALI_BackwardFrame *frame);

SYN_Status syn_dali_slave_init(SYN_DALI_SlaveState *slave, const SYN_DALI_SlaveConfig *cfg);

SYN_Status syn_dali_slave_process(SYN_DALI_SlaveState *slave,
                                  const SYN_DALI_ForwardFrame *req,
                                  uint8_t *resp_data,
                                  bool *has_resp);

size_t syn_dali_manchester_encode_byte(uint8_t val, uint8_t *bit_out);

bool syn_dali_manchester_decode_byte(const uint8_t *bit_in, uint8_t *val_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DALI_H */
```


