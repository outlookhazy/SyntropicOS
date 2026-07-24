

# File syn\_modbus\_master.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_master.h**](syn__modbus__master_8h.md)

[Go to the documentation of this file](syn__modbus__master_8h.md)


```C++

#ifndef SYN_MODBUS_MASTER_H
#define SYN_MODBUS_MASTER_H

#include "../common/syn_defs.h"
#include "syn_modbus.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_MB_MASTER_STATE_IDLE = 0,
    SYN_MB_MASTER_STATE_WAITING_RESPONSE,
    SYN_MB_MASTER_STATE_COMPLETE,
    SYN_MB_MASTER_STATE_TIMEOUT,
    SYN_MB_MASTER_STATE_ERROR,
} SYN_ModbusMaster_State;

typedef struct {
    SYN_ModbusMaster_State state;               
    uint8_t   buf[256];                          
    uint16_t  rx_len;                            
    uint16_t  tx_len;                            
    uint8_t   slave_addr;                        
    uint8_t   func_code;                         
    uint16_t  start_addr;                        
    uint16_t  count;                             
    uint32_t  request_tick_ms;                   
    uint32_t  timeout_ms;                        
    uint32_t  last_byte_tick_ms;                 
    uint8_t   response_fc;                       
    uint8_t   exception_code;                    
    uint16_t  read_data[125];                    
    uint16_t  read_count;                        
} SYN_ModbusMaster;

void syn_modbus_master_init(SYN_ModbusMaster *m, uint32_t timeout_ms);

SYN_Status syn_modbus_master_read_holding(SYN_ModbusMaster *m, uint8_t slave_addr,
                                           uint16_t start_addr, uint16_t count);

SYN_Status syn_modbus_master_read_input(SYN_ModbusMaster *m, uint8_t slave_addr,
                                         uint16_t start_addr, uint16_t count);

SYN_Status syn_modbus_master_write_single(SYN_ModbusMaster *m, uint8_t slave_addr,
                                           uint16_t reg_addr, uint16_t value);

SYN_Status syn_modbus_master_write_multiple(SYN_ModbusMaster *m, uint8_t slave_addr,
                                            uint16_t start_addr, uint16_t count,
                                            const uint16_t *values);

SYN_Status syn_modbus_master_read_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                         uint16_t start_addr, uint16_t count);

SYN_Status syn_modbus_master_read_discrete_inputs(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                   uint16_t start_addr, uint16_t count);

SYN_Status syn_modbus_master_write_single_coil(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                 uint16_t coil_addr, bool state);

SYN_Status syn_modbus_master_write_multiple_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                   uint16_t start_addr, uint16_t count,
                                                   const uint8_t *coil_bytes);

SYN_Status syn_modbus_master_mask_write_register(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                  uint16_t reg_addr, uint16_t and_mask,
                                                  uint16_t or_mask);

SYN_Status syn_modbus_master_read_fifo_queue(SYN_ModbusMaster *m, uint8_t slave_addr,
                                              uint16_t fifo_addr);

SYN_Status syn_modbus_master_report_server_id(SYN_ModbusMaster *m, uint8_t slave_addr);

void syn_modbus_master_feed(SYN_ModbusMaster *m, uint8_t byte);

SYN_ModbusMaster_State syn_modbus_master_process(SYN_ModbusMaster *m, uint32_t current_tick_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_MASTER_H */
```


