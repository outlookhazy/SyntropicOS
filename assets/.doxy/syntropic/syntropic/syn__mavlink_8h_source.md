

# File syn\_mavlink.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mavlink.h**](syn__mavlink_8h.md)

[Go to the documentation of this file](syn__mavlink_8h.md)


```C++

#ifndef SYN_MAVLINK_H
#define SYN_MAVLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_MAVLINK_STX_V2 0xFD          
#define SYN_MAVLINK_MAX_PAYLOAD_LEN 255U 
#define SYN_MAVLINK_HEADER_LEN 10U       
typedef enum {
    SYN_MAVLINK_MSG_HEARTBEAT = 0U,
    SYN_MAVLINK_MSG_SYS_STATUS = 1U,
    SYN_MAVLINK_MSG_ATTITUDE = 30U,
    SYN_MAVLINK_MSG_GLOBAL_POSITION_INT = 33U,
    SYN_MAVLINK_MSG_VFR_HUD = 74U
} SYN_MAVLINK_MsgId;

typedef struct {
    uint8_t payload_len;                          
    uint8_t incompat_flags;                       
    uint8_t compat_flags;                         
    uint8_t seq;                                  
    uint8_t sys_id;                               
    uint8_t comp_id;                              
    uint32_t msg_id;                              
    uint8_t payload[SYN_MAVLINK_MAX_PAYLOAD_LEN]; 
} SYN_MAVLINK_Frame;

typedef struct {
    uint8_t state;                                
    uint8_t payload_len;                          
    uint8_t incompat_flags;                       
    uint8_t compat_flags;                         
    uint8_t seq;                                  
    uint8_t sys_id;                               
    uint8_t comp_id;                              
    uint32_t msg_id;                              
    uint8_t payload_idx;                          
    uint8_t payload[SYN_MAVLINK_MAX_PAYLOAD_LEN]; 
    uint16_t crc;                                 
    SYN_MAVLINK_Frame last_frame;                 
    uint32_t packets_received;                    
    uint32_t crc_errors;                          
} SYN_MAVLINK_Parser;

SYN_Status syn_mavlink_init(SYN_MAVLINK_Parser *parser);

SYN_Status syn_mavlink_parse_byte(SYN_MAVLINK_Parser *parser, uint8_t byte,
                                  SYN_MAVLINK_Frame *frame);

SYN_Status syn_mavlink_encode_msg(uint8_t sys_id, uint8_t comp_id, uint8_t seq, uint32_t msg_id,
                                  uint8_t crc_extra, const uint8_t *payload, uint8_t payload_len,
                                  uint8_t *buf_out, size_t *out_len);

uint16_t syn_mavlink_crc_accumulate(uint8_t byte, uint16_t crc);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MAVLINK_H */
```


