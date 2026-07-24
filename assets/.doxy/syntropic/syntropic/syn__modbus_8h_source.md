

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

#define SYN_MB_FC_READ_COILS            0x01  
#define SYN_MB_FC_READ_DISCRETE_INPUTS  0x02  
#define SYN_MB_FC_READ_HOLDING          0x03  
#define SYN_MB_FC_READ_INPUT            0x04  
#define SYN_MB_FC_WRITE_SINGLE_COIL     0x05  
#define SYN_MB_FC_WRITE_SINGLE          0x06  
#define SYN_MB_FC_READ_EXCEPTION_STATUS 0x07  
#define SYN_MB_FC_DIAGNOSTICS           0x08  
#define SYN_MB_FC_GET_COMM_EVENT_CNT    0x0B  
#define SYN_MB_FC_GET_COMM_EVENT_LOG    0x0C  
#define SYN_MB_FC_WRITE_MULTIPLE_COILS  0x0F  
#define SYN_MB_FC_WRITE_MULTIPLE        0x10  
#define SYN_MB_FC_REPORT_SERVER_ID      0x11  
#define SYN_MB_FC_READ_FILE_RECORD      0x14  
#define SYN_MB_FC_WRITE_FILE_RECORD     0x15  
#define SYN_MB_FC_MASK_WRITE_REGISTER   0x16  
#define SYN_MB_FC_READ_WRITE_MULTIPLE   0x17  
#define SYN_MB_FC_READ_FIFO_QUEUE       0x18  
#define SYN_MB_FC_READ_DEVICE_INFO      0x2B  
#define SYN_MB_MEI_TYPE_READ_DEVICE_ID  0x0E  
#define SYN_MB_EX_ILLEGAL_FUNC    0x01  
#define SYN_MB_EX_ILLEGAL_ADDR    0x02  
#define SYN_MB_EX_ILLEGAL_VALUE   0x03  
#define SYN_MB_EX_DEVICE_FAILURE  0x04  
/* ── Callbacks & Structures ─────────────────────────────────────────────── */

struct SYN_Modbus;

typedef struct {
    const char *vendor_name;     
    const char *product_code;    
    const char *revision;        
    const char *vendor_url;      
    const char *product_name;    
    const char *model_name;      
    const char *user_app_name;   
} SYN_Modbus_DeviceInfo;

typedef bool (*SYN_Modbus_WriteCallback)(struct SYN_Modbus *mb,
                                         uint16_t addr, uint16_t count,
                                         void *ctx);

typedef bool (*SYN_Modbus_ReadFileRecordCallback)(struct SYN_Modbus *mb,
                                                   uint16_t file_num,
                                                   uint16_t record_num,
                                                   uint16_t record_len,
                                                   uint16_t *record_data,
                                                   void *ctx);

typedef bool (*SYN_Modbus_WriteFileRecordCallback)(struct SYN_Modbus *mb,
                                                    uint16_t file_num,
                                                    uint16_t record_num,
                                                    uint16_t record_len,
                                                    const uint16_t *record_data,
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
    /* Discrete bit maps (coils & discrete inputs) */
    uint8_t         *coils;          
    uint16_t         coils_count;    
    uint8_t         *discrete_inputs;
    uint16_t         discrete_count; 
    /* FIFO Queue map */
    const uint16_t  *fifo_queue;     
    uint16_t         fifo_count;     
    /* Optional */
    SYN_Modbus_WriteCallback on_write;    
    void            *on_write_ctx;        
    uint32_t         silence_ms;          
    uint8_t          exception_status;    
    const SYN_Modbus_DeviceInfo *device_info; 
    const uint8_t   *server_id;           
    uint8_t          server_id_len;       
    SYN_Modbus_ReadFileRecordCallback  on_read_file;  
    SYN_Modbus_WriteFileRecordCallback on_write_file; 
    void            *file_cb_ctx;         
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
    uint16_t     comm_event_counter; 
    uint16_t     bus_message_count;   
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


