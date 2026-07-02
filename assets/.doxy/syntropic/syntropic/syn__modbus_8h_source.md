

# File syn\_modbus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus.h**](syn__modbus_8h.md)

[Go to the documentation of this file](syn__modbus_8h.md)


```C++

#ifndef SYN_MODBUS_H
#define SYN_MODBUS_H

#include "../common/syn_defs.h"
#include "../drivers/syn_uart.h"
#include "../util/syn_crc.h"
#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MB_FC_READ_HOLDING    0x03  
#define SYN_MB_FC_READ_INPUT      0x04  
#define SYN_MB_FC_WRITE_SINGLE    0x06  
#define SYN_MB_FC_WRITE_MULTIPLE  0x10  
#define SYN_MB_EX_ILLEGAL_FUNC    0x01  
#define SYN_MB_EX_ILLEGAL_ADDR    0x02  
#define SYN_MB_EX_ILLEGAL_VALUE   0x03  
#define SYN_MB_EX_DEVICE_FAILURE  0x04  
/* ── Callbacks ──────────────────────────────────────────────────────────── */

struct SYN_Modbus;

typedef bool (*SYN_Modbus_WriteCallback)(struct SYN_Modbus *mb,
                                         uint16_t addr, uint16_t count,
                                         void *ctx);

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    uint8_t          slave_addr;     
    SYN_UARTInstance uart;          
    /* Register maps (application-owned) */
    uint16_t        *holding_regs;   
    uint16_t         holding_count;  
    uint16_t        *input_regs;     
    uint16_t         input_count;    
    /* Optional */
    SYN_Modbus_WriteCallback on_write;    
    void            *on_write_ctx;        
} SYN_Modbus_Config;

/* ── Modbus instance ────────────────────────────────────────────────────── */

typedef struct SYN_Modbus {
    SYN_Modbus_Config cfg;           
    /* Frame buffer */
    uint8_t     *buf;            
    uint16_t     buf_size;       
    uint16_t     rx_len;         
    uint32_t     last_byte_tick; 
    bool         frame_ready;    
    /* Stats */
    uint32_t     frames_rx;      
    uint32_t     frames_tx;      
    uint32_t     errors;         
} SYN_Modbus;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_modbus_init(SYN_Modbus *mb, const SYN_Modbus_Config *cfg,
                      uint8_t *buf, uint16_t buf_size);

bool syn_modbus_poll(SYN_Modbus *mb);

void syn_modbus_feed(SYN_Modbus *mb, uint8_t byte);

bool syn_modbus_process(SYN_Modbus *mb);

void syn_modbus_reset(SYN_Modbus *mb);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_H */
```


