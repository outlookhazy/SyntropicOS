

# File syn\_ethercat.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ethercat.h**](syn__ethercat_8h.md)

[Go to the documentation of this file](syn__ethercat_8h.md)


```C++

#ifndef SYN_ETHERCAT_H
#define SYN_ETHERCAT_H

#include "../common/syn_defs.h"
#include "syn_canopen.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_ETHERCAT_ETHERTYPE 0x88A4

#define SYN_ETHERCAT_TYPE_DATAGRAM 1

/* ── EtherCAT Commands ─────────────────────────────────────────────────── */

typedef enum {
    SYN_ECAT_CMD_NOP = 0,   
    SYN_ECAT_CMD_APRD = 1,  
    SYN_ECAT_CMD_APWR = 2,  
    SYN_ECAT_CMD_APRW = 3,  
    SYN_ECAT_CMD_FPRD = 4,  
    SYN_ECAT_CMD_FPWR = 5,  
    SYN_ECAT_CMD_FPRW = 6,  
    SYN_ECAT_CMD_BRD = 7,   
    SYN_ECAT_CMD_BWR = 8,   
    SYN_ECAT_CMD_BRW = 9,   
    SYN_ECAT_CMD_LRD = 10,  
    SYN_ECAT_CMD_LWR = 11,  
    SYN_ECAT_CMD_LRW = 12,  
    SYN_ECAT_CMD_ARMW = 13, 
    SYN_ECAT_CMD_FRMW = 14  
} SYN_EcatCmd;

/* ── EtherCAT State Machine (ESM) States ────────────────────────────────── */

typedef enum {
    SYN_ECAT_STATE_NONE = 0x00,   
    SYN_ECAT_STATE_INIT = 0x01,   
    SYN_ECAT_STATE_PREOP = 0x02,  
    SYN_ECAT_STATE_BOOT = 0x03,   
    SYN_ECAT_STATE_SAFEOP = 0x04, 
    SYN_ECAT_STATE_OP = 0x08      
} SYN_EcatState;

/* ── CoE Mailbox Service Types ──────────────────────────────────────────── */

typedef enum {
    SYN_ECAT_COE_TYPE_EMERGENCY = 1, 
    SYN_ECAT_COE_TYPE_SDO_REQ = 2,   
    SYN_ECAT_COE_TYPE_SDO_RESP = 3,  
    SYN_ECAT_COE_TYPE_RXPDO = 4,     
    SYN_ECAT_COE_TYPE_TXPDO = 5,     
    SYN_ECAT_COE_TYPE_SDO_INFO = 6   
} SYN_EcatCoEType;

/* ── Struct Definitions ─────────────────────────────────────────────────── */

typedef struct {
    uint16_t length : 11;  
    uint16_t reserved : 1; 
    uint16_t type : 4;     
} SYN_EcatHeader;

typedef struct {
    uint8_t cmd;       
    uint8_t idx;       
    uint32_t addr;     
    uint16_t len : 11; 
    uint16_t res : 3;  
    uint16_t circ : 1; 
    uint16_t m : 1;    
    uint16_t irq;      
    uint16_t wkc;      
} SYN_EcatDatagram;

typedef struct {
    uint16_t number : 9;       
    uint16_t res : 3;          
    uint16_t service_type : 4; 
} SYN_EcatCoEHeader;

typedef struct {
    SYN_EcatState state;        
    SYN_EcatState target_state; 
    uint16_t station_addr;      
    uint16_t al_status;         
    uint16_t wkc_expected;      
    uint16_t wkc_last;          
    uint32_t rx_pdos;           
    uint32_t tx_pdos;           
    SYN_CANOpenNode *od;        
} SYN_EcatNode;

/* ── API Functions ──────────────────────────────────────────────────────── */

void syn_ecat_init(SYN_EcatNode *node, uint16_t station_addr, SYN_CANOpenNode *od);

size_t syn_ecat_build_datagram_frame(uint8_t *buf, size_t buf_len, const SYN_EcatDatagram *datagram,
                                     const uint8_t *data, uint16_t data_len);

SYN_Status syn_ecat_parse_frame(SYN_EcatNode *node, const uint8_t *frame, size_t len,
                                uint16_t *wkc);

SYN_Status syn_ecat_set_state(SYN_EcatNode *node, SYN_EcatState new_state);

void syn_ecat_update(SYN_EcatNode *node);

size_t syn_ecat_coe_encode_sdo_download(uint8_t *buf, size_t buf_len, uint16_t index,
                                        uint8_t subindex, const void *data, size_t data_len);

size_t syn_ecat_coe_encode_sdo_upload(uint8_t *buf, size_t buf_len, uint16_t index,
                                      uint8_t subindex);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ETHERCAT_H */
```


