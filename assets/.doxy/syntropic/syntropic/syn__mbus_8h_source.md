

# File syn\_mbus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mbus.h**](syn__mbus_8h.md)

[Go to the documentation of this file](syn__mbus_8h.md)


```C++

#ifndef SYN_MBUS_H
#define SYN_MBUS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MBUS) || SYN_USE_MBUS

#ifdef __cplusplus
extern "C" {
#endif

/* ── M-Bus Delimiters & Constants ────────────────────────────────────────── */

#define SYN_MBUS_START_SHORT 0x10u   
#define SYN_MBUS_START_CONTROL 0x68u 
#define SYN_MBUS_START_LONG 0x68u    
#define SYN_MBUS_STOP 0x16u          
#define SYN_MBUS_ACK_BYTE 0xE5u      
/* ── Standard M-Bus Control (C) Field Codes ─────────────────────────────── */

#define SYN_MBUS_C_SND_NKE 0x40u     
#define SYN_MBUS_C_SND_UD 0x53u      
#define SYN_MBUS_C_SND_UD_FCB 0x73u  
#define SYN_MBUS_C_REQ_UD2 0x5Bu     
#define SYN_MBUS_C_REQ_UD2_FCB 0x7Bu 
#define SYN_MBUS_C_REQ_UD1 0x5Au     
#define SYN_MBUS_C_RSP_UD 0x08u      
/* ── Standard M-Bus Control Information (CI) Field Codes ───────────────── */

#define SYN_MBUS_CI_RSP_DATA_LSB 0x72u 
#define SYN_MBUS_CI_RSP_DATA_MSB 0x73u 
#define SYN_MBUS_CI_SND_UD_LSB 0x51u   
#define SYN_MBUS_CI_SELECT_SLAVE 0x52u 
/* ── M-Bus Special Addresses ────────────────────────────────────────────── */

#define SYN_MBUS_ADDR_BROADCAST_REPLY 0xFEu 
#define SYN_MBUS_ADDR_BROADCAST_NONE 0xFFu  
/* ── Limits ─────────────────────────────────────────────────────────────── */

#define SYN_MBUS_MAX_PAYLOAD 252u 
#define SYN_MBUS_MAX_FRAME_LEN \
    261u 
/* ── M-Bus Frame Types ─────────────────────────────────────────────────── */

typedef enum {
    SYN_MBUS_FRAME_TYPE_UNKNOWN = 0, 
    SYN_MBUS_FRAME_TYPE_SINGLE_ACK,  
    SYN_MBUS_FRAME_TYPE_SHORT,       
    SYN_MBUS_FRAME_TYPE_CONTROL,     
    SYN_MBUS_FRAME_TYPE_LONG,        
} SYN_MBUS_FrameType;

/* ── M-Bus Decoded Frame Structure ──────────────────────────────────────── */

typedef struct {
    SYN_MBUS_FrameType type;               
    uint8_t c_field;                       
    uint8_t a_field;                       
    uint8_t ci_field;                      
    uint8_t payload[SYN_MBUS_MAX_PAYLOAD]; 
    uint8_t payload_len;                   
    uint8_t checksum;                      
    bool checksum_valid;                   
} SYN_MBUS_Frame;

/* ── Streaming Decoder Callback ─────────────────────────────────────────── */

typedef void (*SYN_MBUS_FrameCallback)(const SYN_MBUS_Frame *frame, void *ctx);

typedef struct {
    uint8_t rx_buf[SYN_MBUS_MAX_FRAME_LEN]; 
    size_t rx_idx;                          
    size_t expected_len;                    
    uint8_t state;                          
    SYN_MBUS_FrameCallback callback;        
    void *ctx;                              
} SYN_MBUS_Decoder;

/* ── API Function Declarations ───────────────────────────────────────────── */

uint8_t syn_mbus_calc_checksum(const uint8_t *data, size_t len);

SYN_Status syn_mbus_encode_ack(uint8_t *buf, size_t cap, size_t *out_len);

SYN_Status syn_mbus_encode_short(uint8_t c_field, uint8_t a_field, uint8_t *buf, size_t cap,
                                 size_t *out_len);

SYN_Status syn_mbus_encode_control(uint8_t c_field, uint8_t a_field, uint8_t ci_field, uint8_t *buf,
                                   size_t cap, size_t *out_len);

SYN_Status syn_mbus_encode_long(uint8_t c_field, uint8_t a_field, uint8_t ci_field,
                                const uint8_t *payload, uint8_t payload_len, uint8_t *buf,
                                size_t cap, size_t *out_len);

SYN_Status syn_mbus_decode_frame(const uint8_t *buf, size_t len, SYN_MBUS_Frame *frame);

void syn_mbus_decoder_init(SYN_MBUS_Decoder *dec, SYN_MBUS_FrameCallback callback, void *ctx);

void syn_mbus_decoder_reset(SYN_MBUS_Decoder *dec);

void syn_mbus_decoder_feed(SYN_MBUS_Decoder *dec, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MBUS */
#endif /* SYN_MBUS_H */
```


