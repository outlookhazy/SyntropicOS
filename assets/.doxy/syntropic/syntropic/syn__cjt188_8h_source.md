

# File syn\_cjt188.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cjt188.h**](syn__cjt188_8h.md)

[Go to the documentation of this file](syn__cjt188_8h.md)


```C++

#ifndef SYN_CJT188_H
#define SYN_CJT188_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Meter Types (T) ────────────────────────────────────────────────────── */

#define SYN_CJT188_METER_COLD_WATER 0x10U      
#define SYN_CJT188_METER_HOT_WATER 0x11U       
#define SYN_CJT188_METER_DRINK_WATER 0x12U     
#define SYN_CJT188_METER_RECLAIMED_WATER 0x13U 
#define SYN_CJT188_METER_HEAT 0x20U            
#define SYN_CJT188_METER_COOLING 0x21U         
#define SYN_CJT188_METER_GAS 0x30U             
#define SYN_CJT188_METER_POWER 0x40U           
/* ── Control Codes (C) ───────────────────────────────────────────────────── */

#define SYN_CJT188_CTRL_READ_DATA 0x01U       
#define SYN_CJT188_CTRL_READ_DATA_RESP 0x81U  
#define SYN_CJT188_CTRL_WRITE_DATA 0x04U      
#define SYN_CJT188_CTRL_WRITE_DATA_RESP 0x84U 
#define SYN_CJT188_CTRL_WRITE_ADDR 0x15U      
#define SYN_CJT188_CTRL_WRITE_ADDR_RESP 0x95U 
/* ── Data Identifiers (DI) ──────────────────────────────────────────────── */

#define SYN_CJT188_DI_READ_METER_DATA 0x901FU 
#define SYN_CJT188_DI_READ_HIST_DATA 0xD120U  
#define SYN_CJT188_DI_VALVE_CONTROL 0xA017U   
/* ── Valve Control Function Codes ───────────────────────────────────────── */

#define SYN_CJT188_VALVE_OPEN 0x55U  
#define SYN_CJT188_VALVE_CLOSE 0x99U 
/* ── Framing Delimiters ─────────────────────────────────────────────────── */

#define SYN_CJT188_PREAMBLE_BYTE 0xFEU 
#define SYN_CJT188_START_BYTE 0x68U    
#define SYN_CJT188_END_BYTE 0x16U      
#define SYN_CJT188_MIN_FRAME_SIZE 13U  
#define SYN_CJT188_MAX_FRAME_SIZE 128U 
/* ── Data Structures ────────────────────────────────────────────────────── */

typedef struct {
    uint8_t meter_type;     
    uint8_t meter_id[5];    
    uint8_t vendor_id[2];   
    uint8_t ctrl;           
    uint8_t len;            
    uint16_t data_id;       
    uint8_t seq;            
    const uint8_t *payload; 
    size_t payload_len;     
} SYN_CJT188_Frame;

typedef struct {
    uint32_t current_flow_bcd; 
    uint8_t unit;              
    uint32_t month_flow_bcd;   
    uint16_t status;           
} SYN_CJT188_MeterData;

typedef struct {
    uint8_t buf[SYN_CJT188_MAX_FRAME_SIZE]; 
    size_t index;                           
    size_t expected_len;                    
    bool in_frame;                          
} SYN_CJT188_Decoder;

/* ── Public API ─────────────────────────────────────────────────────────── */

uint8_t syn_cjt188_checksum(const uint8_t *buf, size_t len);

size_t syn_cjt188_encode_read_req(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                  const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                  uint16_t data_id, uint8_t seq);

size_t syn_cjt188_encode_valve_ctrl(uint8_t *out_buf, size_t buf_size, uint8_t meter_type,
                                    const uint8_t meter_id[5], const uint8_t vendor_id[2],
                                    bool open_valve, uint8_t seq);

bool syn_cjt188_parse_frame(const uint8_t *buf, size_t len, SYN_CJT188_Frame *out_frame);

void syn_cjt188_decoder_init(SYN_CJT188_Decoder *decoder);

bool syn_cjt188_decoder_feed(SYN_CJT188_Decoder *decoder, uint8_t byte,
                             SYN_CJT188_Frame *out_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CJT188_H */
```


