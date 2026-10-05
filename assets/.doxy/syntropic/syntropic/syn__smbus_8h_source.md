

# File syn\_smbus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_smbus.h**](syn__smbus_8h.md)

[Go to the documentation of this file](syn__smbus_8h.md)


```C++

#ifndef SYN_SMBUS_H
#define SYN_SMBUS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SMBUS) || SYN_USE_SMBUS

#ifdef __cplusplus
extern "C" {
#endif

/* ── Standard SMBus Slave Addresses ─────────────────────────────────────── */

#define SYN_SMBUS_ADDR_ALERT_RESPONSE 0x0C        
#define SYN_SMBUS_ADDR_SMART_BATTERY_CHARGER 0x12 
#define SYN_SMBUS_ADDR_SMART_BATTERY_SELECT 0x13  
#define SYN_SMBUS_ADDR_SMART_BATTERY 0x16         
#define SYN_SMBUS_ADDR_DEFAULT 0x61               
/* ── SMBus Limits ───────────────────────────────────────────────────────── */

#define SYN_SMBUS_BLOCK_MAX_LEN 32 
#define SYN_SMBUS_BUF_MAX_LEN 260  
/* ── SMBus Transaction Protocol Types ────────────────────────────────────── */

typedef enum {
    SYN_SMBUS_PROTO_QUICK_READ = 0,    
    SYN_SMBUS_PROTO_QUICK_WRITE,       
    SYN_SMBUS_PROTO_SEND_BYTE,         
    SYN_SMBUS_PROTO_RECEIVE_BYTE,      
    SYN_SMBUS_PROTO_WRITE_BYTE,        
    SYN_SMBUS_PROTO_READ_BYTE,         
    SYN_SMBUS_PROTO_WRITE_WORD,        
    SYN_SMBUS_PROTO_READ_WORD,         
    SYN_SMBUS_PROTO_WRITE_BLOCK,       
    SYN_SMBUS_PROTO_READ_BLOCK,        
    SYN_SMBUS_PROTO_PROCESS_CALL,      
    SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL 
} SYN_SMBUS_Protocol;

/* ── SMBus Packet Structure ──────────────────────────────────────────────── */

typedef struct {
    uint8_t slave_addr;                  
    uint8_t command;                     
    SYN_SMBUS_Protocol proto;            
    bool pec_enabled;                    
    uint8_t length;                      
    uint8_t data[SYN_SMBUS_BUF_MAX_LEN]; 
    uint8_t pec;                         
    bool pec_valid;                      
} SYN_SMBUS_Packet;

/* ── API Function Declarations ───────────────────────────────────────────── */

uint8_t syn_smbus_calc_pec(uint8_t init_crc, const uint8_t *data, size_t len);

SYN_Status syn_smbus_encode_packet(const SYN_SMBUS_Packet *pkt, uint8_t *tx_buf, size_t buf_size,
                                   size_t *out_len);

SYN_Status syn_smbus_decode_packet(SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf, size_t rx_len,
                                   SYN_SMBUS_Protocol proto, bool has_pec);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SMBUS */
#endif /* SYN_SMBUS_H */
```


