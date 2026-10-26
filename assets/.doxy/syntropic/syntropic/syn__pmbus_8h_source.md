

# File syn\_pmbus.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_pmbus.h**](syn__pmbus_8h.md)

[Go to the documentation of this file](syn__pmbus_8h.md)


```C++

#ifndef SYN_PMBUS_H
#define SYN_PMBUS_H

#include "../common/syn_defs.h"
#include "syn_smbus.h"

#include <stdbool.h>
#include <stdint.h>

#if !defined(SYN_USE_PMBUS) || SYN_USE_PMBUS

#ifdef __cplusplus
extern "C" {
#endif

/* ── PMBus Command Codes ────────────────────────────────────────────────── */

#define SYN_PMBUS_CMD_PAGE 0x00                 
#define SYN_PMBUS_CMD_OPERATION 0x01            
#define SYN_PMBUS_CMD_ON_OFF_CONFIG 0x02        
#define SYN_PMBUS_CMD_CLEAR_FAULTS 0x03         
#define SYN_PMBUS_CMD_PHASE 0x04                
#define SYN_PMBUS_CMD_WRITE_PROTECT 0x10        
#define SYN_PMBUS_CMD_STORE_DEFAULT_ALL 0x11    
#define SYN_PMBUS_CMD_RESTORE_DEFAULT_ALL 0x12  
#define SYN_PMBUS_CMD_CAPABILITY 0x19           
#define SYN_PMBUS_CMD_VOUT_MODE 0x20            
#define SYN_PMBUS_CMD_VOUT_COMMAND 0x21         
#define SYN_PMBUS_CMD_VOUT_MAX 0x24             
#define SYN_PMBUS_CMD_VOUT_MARGIN_HIGH 0x25     
#define SYN_PMBUS_CMD_VOUT_MARGIN_LOW 0x26      
#define SYN_PMBUS_CMD_VOUT_TRANSITION_RATE 0x27 
#define SYN_PMBUS_CMD_VOUT_DROOP 0x28           
#define SYN_PMBUS_CMD_STATUS_BYTE 0x78          
#define SYN_PMBUS_CMD_STATUS_WORD 0x79          
#define SYN_PMBUS_CMD_STATUS_VOUT 0x7A          
#define SYN_PMBUS_CMD_STATUS_IOUT 0x7B          
#define SYN_PMBUS_CMD_STATUS_INPUT 0x7C         
#define SYN_PMBUS_CMD_STATUS_TEMPERATURE 0x7D   
#define SYN_PMBUS_CMD_STATUS_CBUFFER 0x7E       
#define SYN_PMBUS_CMD_STATUS_OTHER 0x7F         
#define SYN_PMBUS_CMD_STATUS_MFR_SPECIFIC 0x80  
#define SYN_PMBUS_CMD_STATUS_FANS_1_2 0x81      
#define SYN_PMBUS_CMD_READ_VIN 0x88             
#define SYN_PMBUS_CMD_READ_IIN 0x89             
#define SYN_PMBUS_CMD_READ_VOUT 0x8B            
#define SYN_PMBUS_CMD_READ_IOUT 0x8C            
#define SYN_PMBUS_CMD_READ_TEMPERATURE_1 0x8D   
#define SYN_PMBUS_CMD_READ_TEMPERATURE_2 0x8E   
#define SYN_PMBUS_CMD_READ_FAN_SPEED_1 0x90     
#define SYN_PMBUS_CMD_READ_DUTY_CYCLE 0x94      
#define SYN_PMBUS_CMD_READ_FREQUENCY 0x95       
#define SYN_PMBUS_CMD_READ_POUT 0x96            
#define SYN_PMBUS_CMD_READ_PIN 0x97             
#define SYN_PMBUS_CMD_PMBUS_REVISION 0x98       
/* ── PMBus Status Bitmask Definitions ───────────────────────────────────── */

#define SYN_PMBUS_STATUS_BYTE_BUSY (1u << 7)          
#define SYN_PMBUS_STATUS_BYTE_OFF (1u << 6)           
#define SYN_PMBUS_STATUS_BYTE_VOUT_OV (1u << 5)       
#define SYN_PMBUS_STATUS_BYTE_IOUT_OC (1u << 4)       
#define SYN_PMBUS_STATUS_BYTE_VIN_UV (1u << 3)        
#define SYN_PMBUS_STATUS_BYTE_TEMP_FAULT (1u << 2)    
#define SYN_PMBUS_STATUS_BYTE_CBUF_FAULT (1u << 1)    
#define SYN_PMBUS_STATUS_BYTE_NONE_OF_ABOVE (1u << 0) 
/* ── API Function Declarations ───────────────────────────────────────────── */

float syn_pmbus_linear11_to_float(uint16_t raw);

uint16_t syn_pmbus_float_to_linear11(float val);

float syn_pmbus_linear16_to_float(uint16_t raw, uint8_t vout_mode);

uint16_t syn_pmbus_float_to_linear16(float val, uint8_t vout_mode);

void syn_pmbus_encode_read_cmd(SYN_SMBUS_Packet *pkt, uint8_t slave_addr, uint8_t cmd,
                               bool use_pec);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_PMBUS */
#endif /* SYN_PMBUS_H */
```


