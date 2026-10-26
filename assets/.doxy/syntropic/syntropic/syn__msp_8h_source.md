

# File syn\_msp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_msp.h**](syn__msp_8h.md)

[Go to the documentation of this file](syn__msp_8h.md)


```C++

#ifndef SYN_MSP_H
#define SYN_MSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYN_MSP_MAX_PAYLOAD 64U 
#define SYN_MSP_HEADER_CHAR '$' 
#define SYN_MSP_V1_CHAR 'M'     
#define SYN_MSP_REQ_CHAR '<'    
#define SYN_MSP_RESP_CHAR '>'   
#define SYN_MSP_ERR_CHAR '!'    
typedef enum {
    SYN_MSP_API_VERSION = 1U,
    SYN_MSP_FC_VARIANT = 2U,
    SYN_MSP_STATUS = 101U,
    SYN_MSP_RAW_IMU = 102U,
    SYN_MSP_ATTITUDE = 108U,
    SYN_MSP_ALTITUDE = 109U,
    SYN_MSP_BATTERY_STATE = 130U
} SYN_MSP_Cmd;

typedef struct {
    uint8_t dir_char;                     
    uint8_t cmd;                          
    uint8_t payload_len;                  
    uint8_t payload[SYN_MSP_MAX_PAYLOAD]; 
} SYN_MSP_Frame;

typedef struct {
    uint8_t state;                        
    uint8_t dir_char;                     
    uint8_t cmd;                          
    uint8_t payload_len;                  
    uint8_t payload_idx;                  
    uint8_t payload[SYN_MSP_MAX_PAYLOAD]; 
    uint8_t checksum;                     
    SYN_MSP_Frame last_frame;             
    uint32_t frames_received;             
    uint32_t checksum_errors;             
} SYN_MSP_Parser;

SYN_Status syn_msp_init(SYN_MSP_Parser *parser);

SYN_Status syn_msp_parse_byte(SYN_MSP_Parser *parser, uint8_t byte, SYN_MSP_Frame *frame);

SYN_Status syn_msp_encode_response(uint8_t cmd, const uint8_t *payload, uint8_t len,
                                   uint8_t *buf_out, size_t buf_size, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MSP_H */
```


