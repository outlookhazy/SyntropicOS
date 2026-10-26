

# File syn\_ethercat.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ethercat.h**](syn__ethercat_8h.md)

[Go to the documentation of this file](syn__ethercat_8h.md)


```C++

#ifndef SYN_ETHERCAT_H
#define SYN_ETHERCAT_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
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

/* ── EtherCAT Master & Configuration Structs ────────────────────────────── */

#ifndef SYN_ECAT_MAX_SLAVES
#define SYN_ECAT_MAX_SLAVES 16U 
#endif

typedef struct {
    uint16_t start_addr; 
    uint16_t length;     
    uint8_t control;     
    uint8_t enable;      
} SYN_EcatSMConfig;

typedef struct {
    uint32_t logical_start;    
    uint16_t length;           
    uint8_t logical_start_bit; 
    uint8_t logical_end_bit;   
    uint16_t phys_start_addr;  
    uint8_t phys_start_bit;    
    uint8_t type;              
    bool enable;               
} SYN_EcatFMMUConfig;

typedef struct {
    uint16_t station_addr;   
    uint16_t auto_inc_addr;  
    uint32_t vendor_id;      
    uint32_t product_code;   
    uint32_t revision;       
    SYN_EcatState state;     
    uint16_t al_status_code; 
    uint8_t sm_count;        
    uint8_t fmmu_count;      
} SYN_EcatSlaveInfo;

typedef struct {
    SYN_EcatSlaveInfo slaves[SYN_ECAT_MAX_SLAVES]; 
    uint8_t slave_count;                           
    uint8_t *output_image; 
    uint8_t *input_image;  
    uint16_t output_size;  
    uint16_t input_size;   
    uint32_t logical_addr; 
    uint16_t wkc_expected; 
    uint16_t wkc_last;     
    uint32_t cycle_count;  
    uint32_t wkc_errors;   
    uint8_t *tx_buf;      
    uint8_t *rx_buf;      
    uint16_t tx_buf_size; 
    uint16_t rx_buf_size; 
    /* Transport & Protothread Task State */
    SYN_EcatState master_state; 
    uint8_t current_slave_idx;  
    uint16_t tx_frame_len;      
    uint16_t rx_frame_len;      
    bool frame_tx_pending;      
    bool frame_rx_ready;        
    /* Scratch state for non-blocking task drivers & hardware polling */
    SYN_PT sub_pt;          
    uint8_t pdo_count;      
    uint8_t pdo_entry_idx;  
    uint16_t pdo_map_idx;   
    uint32_t pdo_sdo_val;   
    size_t pdo_read_len;    
    uint32_t total_rx_bits; 
    uint32_t total_tx_bits; 
    uint16_t poll_retries;  
} SYN_EcatMaster;

/* ── EtherCAT Master Transport & Protothread Tasks ─────────────────────── */

#include "../sched/syn_task.h"

SYN_Status syn_ecat_master_set_rx_frame(SYN_EcatMaster *m, const uint8_t *frame, size_t len);

bool syn_ecat_master_pop_tx_frame(SYN_EcatMaster *m, const uint8_t **out_frame, size_t *out_len);

SYN_PT_Status syn_ecat_master_scan_task(SYN_PT *pt, SYN_EcatMaster *m);

SYN_PT_Status syn_ecat_master_transition_task(SYN_PT *pt, SYN_EcatMaster *m,
                                              SYN_EcatState target_state);

SYN_PT_Status syn_ecat_master_cyclic_task(SYN_PT *pt, SYN_EcatMaster *m);

size_t syn_ecat_encode_read_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                uint16_t len);

size_t syn_ecat_encode_write_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                 const void *data, uint16_t len);

SYN_Status syn_ecat_decode_read_reg(SYN_EcatMaster *m, size_t rx_len, void *out_data, uint16_t len);

SYN_PT_Status syn_ecat_master_read_sii_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t word_offset, uint32_t *out_data);

SYN_PT_Status syn_ecat_master_sdo_read_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t index, uint8_t subindex, void *out_data,
                                            size_t max_len, size_t *out_len);

SYN_PT_Status syn_ecat_master_discover_pdo_mapping_task(SYN_PT *pt, SYN_EcatMaster *m,
                                                        uint16_t station_addr,
                                                        uint16_t *out_rx_bytes,
                                                        uint16_t *out_tx_bytes);

/* ── EtherCAT Master API ────────────────────────────────────────────────── */

SYN_Status syn_ecat_master_init(SYN_EcatMaster *m, uint8_t *tx_buf, uint16_t tx_buf_size,
                                uint8_t *rx_buf, uint16_t rx_buf_size, uint8_t *output_image,
                                uint16_t output_size, uint8_t *input_image, uint16_t input_size);

size_t syn_ecat_encode_scan_bus(SYN_EcatMaster *m);

uint8_t syn_ecat_decode_scan_bus(SYN_EcatMaster *m, size_t rx_len);

size_t syn_ecat_encode_assign_addr(SYN_EcatMaster *m, uint16_t position, uint16_t station_addr);

size_t syn_ecat_encode_read_sii(SYN_EcatMaster *m, uint16_t station_addr, uint16_t word_offset);

SYN_Status syn_ecat_decode_read_sii(SYN_EcatMaster *m, size_t rx_len, uint32_t *out_val);

size_t syn_ecat_encode_write_sm(SYN_EcatMaster *m, uint16_t station_addr, uint8_t sm_index,
                                const SYN_EcatSMConfig *cfg);

size_t syn_ecat_encode_write_fmmu(SYN_EcatMaster *m, uint16_t station_addr, uint8_t fmmu_index,
                                  const SYN_EcatFMMUConfig *cfg);

size_t syn_ecat_encode_read_al_status(SYN_EcatMaster *m, uint16_t station_addr);

SYN_Status syn_ecat_decode_read_al_status(SYN_EcatMaster *m, size_t rx_len, SYN_EcatState *state,
                                          uint16_t *status_code);

/* ── Multi-Datagram Frame Builder & Parser ─────────────────────────────── */

#define SYN_ESC_REG_TYPE 0x0000U             
#define SYN_ESC_REG_STATION_ADDR 0x0010U     
#define SYN_ESC_REG_STATION_ALIAS 0x0012U    
#define SYN_ESC_REG_AL_CONTROL 0x0120U       
#define SYN_ESC_REG_AL_STATUS 0x0130U        
#define SYN_ESC_REG_AL_STATUS_CODE 0x0134U   
#define SYN_ESC_REG_PDI_CONTROL 0x0140U      
#define SYN_ESC_REG_SII_EEPROM_CTRL 0x0502U  
#define SYN_ESC_REG_FMMU0 0x0600U            
#define SYN_ESC_REG_SM0 0x0800U              
#define SYN_ESC_REG_DC_SYSTEM_TIME 0x0900U   
#define SYN_ESC_REG_DC_SYSTEM_OFFSET 0x0920U 
#define SYN_ESC_REG_DC_SYNC0_CYCLE 0x09A0U   
/* ── Multi-Datagram Frame Builder & Parser ─────────────────────────────── */

typedef struct {
    uint8_t cmd;         
    uint8_t idx;         
    uint32_t addr;       
    const uint8_t *data; 
    uint16_t data_len;   
    uint16_t wkc;        
} SYN_EcatDatagramResult;

void syn_ecat_frame_begin(uint8_t *buf, size_t buf_size);

size_t syn_ecat_frame_add_datagram(uint8_t *buf, size_t buf_size, SYN_EcatCmd cmd, uint8_t idx,
                                   uint32_t addr, const uint8_t *data, uint16_t data_len,
                                   bool circ);

size_t syn_ecat_frame_finalize(uint8_t *buf);

bool syn_ecat_frame_parse_next(const uint8_t *frame, size_t frame_len, size_t *offset,
                               SYN_EcatDatagramResult *out);

size_t syn_ecat_coe_encode_sdo_download(uint8_t *buf, size_t buf_len, uint16_t index,
                                        uint8_t subindex, const void *data, size_t data_len);

size_t syn_ecat_coe_encode_sdo_upload(uint8_t *buf, size_t buf_len, uint16_t index,
                                      uint8_t subindex);

/* ── Phase 3: Mailbox & CoE SDO API ─────────────────────────────────────── */

size_t syn_ecat_encode_coe_sdo_write(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                     uint8_t subindex, const void *data, size_t data_len);

size_t syn_ecat_encode_coe_sdo_read(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                    uint8_t subindex);

SYN_Status syn_ecat_decode_coe_sdo_response(SYN_EcatMaster *m, size_t rx_len, void *out_data,
                                            size_t max_len, size_t *out_len);

/* ── Phase 4: Cyclic Process Data Exchange API ──────────────────────────── */

size_t syn_ecat_encode_cyclic(SYN_EcatMaster *m);

SYN_Status syn_ecat_decode_cyclic(SYN_EcatMaster *m, size_t rx_len);

/* ── Phase 5: ESM State Transitions ─────────────────────────────────────── */

size_t syn_ecat_encode_set_al_control(SYN_EcatMaster *m, uint16_t station_addr,
                                      SYN_EcatState state);

/* ── Phase 6: Distributed Clocks (DC) API ───────────────────────────────── */

size_t syn_ecat_encode_dc_read_system_time(SYN_EcatMaster *m);

size_t syn_ecat_encode_dc_configure(SYN_EcatMaster *m, uint16_t station_addr,
                                    uint32_t sync0_cycle_ns, int32_t sync0_shift_ns);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ETHERCAT_H */
```


