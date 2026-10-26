

# File syn\_dlt645.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dlt645.h**](syn__dlt645_8h.md)

[Go to the documentation of this file](syn__dlt645_8h.md)


```C++

#ifndef SYN_DLT645_H
#define SYN_DLT645_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Protocol Constants ─────────────────────────────────────────────────── */

#define SYN_DLT645_SOF 0x68      
#define SYN_DLT645_EOF 0x16      
#define SYN_DLT645_OFFSET 0x33   
#define SYN_DLT645_PREAMBLE 0xFE 
#define SYN_DLT645_ADDR_LEN 6 
/* ── Protocol Versions ──────────────────────────────────────────────────── */

typedef enum {
    SYN_DLT645_VER_1997 = 0, 
    SYN_DLT645_VER_2007 = 1  
} SYN_DLT645_Ver;

/* ── Control Codes ──────────────────────────────────────────────────────── */

typedef enum {
    SYN_DLT645_CMD_READ_DATA = 0x11,        
    SYN_DLT645_CMD_READ_DATA_RESP = 0x91,   
    SYN_DLT645_CMD_WRITE_DATA = 0x14,       
    SYN_DLT645_CMD_WRITE_DATA_RESP = 0x94,  
    SYN_DLT645_CMD_READ_ADDR = 0x13,        
    SYN_DLT645_CMD_READ_ADDR_RESP = 0x93,   
    SYN_DLT645_CMD_WRITE_ADDR = 0x15,       
    SYN_DLT645_CMD_WRITE_ADDR_RESP = 0x95,  
    SYN_DLT645_CMD_CHANGE_BAUD = 0x17,      
    SYN_DLT645_CMD_CHANGE_BAUD_RESP = 0x97, 
    SYN_DLT645_CMD_ERROR_RESP = 0xD1        
} SYN_DLT645_Cmd;

/* ── Frame Structure ────────────────────────────────────────────────────── */

typedef struct {
    uint8_t address[SYN_DLT645_ADDR_LEN]; 
    uint8_t control;                      
    uint32_t data_id;                     
    uint8_t payload[64];                  
    uint8_t payload_len;                  
    SYN_DLT645_Ver version;               
} SYN_DLT645_Frame;

/* ── Streaming Decoder ──────────────────────────────────────────────────── */

typedef void (*SYN_DLT645_FrameCallback)(const SYN_DLT645_Frame *frame, void *ctx);

typedef struct {
    uint8_t rx_buf[128];         
    size_t rx_len;               
    SYN_DLT645_FrameCallback cb; 
    void *cb_ctx;                
    SYN_DLT645_Ver version;      
} SYN_DLT645_Decoder;

/* ── API ────────────────────────────────────────────────────────────────── */

uint8_t syn_dlt645_calc_checksum(const uint8_t *buf, size_t len);

size_t syn_dlt645_encode(const SYN_DLT645_Frame *frame, uint8_t *out_buf, size_t out_capacity);

SYN_Status syn_dlt645_parse(const uint8_t *in_buf, size_t len, SYN_DLT645_Ver version,
                            SYN_DLT645_Frame *out_frame);

void syn_dlt645_decoder_init(SYN_DLT645_Decoder *dec, SYN_DLT645_Ver version,
                             SYN_DLT645_FrameCallback cb, void *ctx);

void syn_dlt645_decoder_feed(SYN_DLT645_Decoder *dec, uint8_t rx_byte);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DLT645_H */
```


