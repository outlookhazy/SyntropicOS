

# File syn\_ir.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ir.h**](syn__ir_8h.md)

[Go to the documentation of this file](syn__ir_8h.md)


```C++

#ifndef SYN_IR_H
#define SYN_IR_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_IR) || SYN_USE_IR

#ifdef __cplusplus
extern "C" {
#endif

/* ── Encoding Types ────────────────────────────────────────────────────── */

typedef enum {
    SYN_IR_ENC_PDM = 0,        
    SYN_IR_ENC_PWM = 1,        
    SYN_IR_ENC_MANCHESTER = 2, 
    SYN_IR_ENC_PPM = 3         
} SYN_IR_EncodingType;

/* ── Protocol Identifiers ───────────────────────────────────────────────── */

typedef enum {
    SYN_IR_PROTO_UNKNOWN = 0,
    SYN_IR_PROTO_NEC,          
    SYN_IR_PROTO_NEC_EXTENDED, 
    SYN_IR_PROTO_SONY_12,      
    SYN_IR_PROTO_SONY_15,      
    SYN_IR_PROTO_SONY_20,      
    SYN_IR_PROTO_RC5,          
    SYN_IR_PROTO_RC6,          
    SYN_IR_PROTO_SAMSUNG,      
    SYN_IR_PROTO_KASEIKYO,     
    SYN_IR_PROTO_DENON,        
    SYN_IR_PROTO_APPLE,        
    SYN_IR_PROTO_COUNT
} SYN_IR_Protocol;

/* ── Frame Flags ────────────────────────────────────────────────────────── */

#define SYN_IR_FLAG_NONE 0x0000U     
#define SYN_IR_FLAG_REPEAT (1U << 0) 
#define SYN_IR_FLAG_TOGGLE (1U << 1) 
/* ── Decoded IR Frame ───────────────────────────────────────────────────── */

typedef struct {
    SYN_IR_Protocol protocol; 
    uint32_t address;         
    uint32_t command;         
    uint16_t flags;           
    uint8_t bit_count;        
    uint8_t carrier_khz;      
} SYN_IR_Frame;

/* ── Timing Pulse Representation (Encoder Output) ────────────────────────── */

typedef struct {
    uint16_t duration_us; 
    bool is_mark;         
} SYN_IR_Pulse;

/* ── Decoder State Machine Handle ───────────────────────────────────────── */

typedef enum {
    SYN_IR_STATE_IDLE = 0, 
    SYN_IR_STATE_LEADER,   
    SYN_IR_STATE_DATA,     
    SYN_IR_STATE_TRAILER
} SYN_IR_FsmState;

typedef struct {
    SYN_IR_FsmState state;        
    SYN_IR_Protocol active_proto; 
    uint64_t bits;                
    uint8_t bit_idx;              
    uint8_t expected_bits;        
    uint16_t last_mark_us;        
    uint16_t last_space_us;       
    SYN_IR_Frame last_frame;      
    bool have_last;               
    bool manchester_phase;        
} SYN_IR_Decoder;

/* ── API Functions ──────────────────────────────────────────────────────── */

SYN_Status syn_ir_decoder_init(SYN_IR_Decoder *decoder);

bool syn_ir_decode_pulse(SYN_IR_Decoder *decoder, uint16_t duration_us, bool is_mark,
                         SYN_IR_Frame *frame_out);

bool syn_ir_decode_timeout(SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out);

SYN_Status syn_ir_encode_frame(const SYN_IR_Frame *frame, SYN_IR_Pulse *pulse_buf, size_t buf_len,
                               size_t *count_out);

const char *syn_ir_protocol_name(SYN_IR_Protocol proto);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_IR */

#endif /* SYN_IR_H */
```


