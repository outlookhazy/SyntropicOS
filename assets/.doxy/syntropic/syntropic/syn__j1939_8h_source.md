

# File syn\_j1939.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_j1939.h**](syn__j1939_8h.md)

[Go to the documentation of this file](syn__j1939_8h.md)


```C++

#ifndef SYN_J1939_H
#define SYN_J1939_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_J1939_ADDR_GLOBAL        0xFFU 
#define SYN_J1939_ADDR_NULL          0xFEU 
#define SYN_J1939_PGN_REQUEST        59904U 
#define SYN_J1939_PGN_TP_DT          60160U 
#define SYN_J1939_PGN_TP_CM          60416U 
#define SYN_J1939_PGN_ADDR_CLAIM     60928U 
#define SYN_J1939_PGN_PROPRIETARY_A  61184U 
#define SYN_J1939_PGN_EEC1           61444U 
#define SYN_J1939_PGN_DM1            65226U 
#define SYN_J1939_PGN_BMS            65251U 
#define SYN_J1939_PGN_ET1            65262U 
#define SYN_J1939_PGN_PROPRIETARY_B  65280U 
#define SYN_J1939_TP_CTRL_RTS        16U  
#define SYN_J1939_TP_CTRL_CTS        17U  
#define SYN_J1939_TP_CTRL_ACK        19U  
#define SYN_J1939_TP_CTRL_BAM        32U  
#define SYN_J1939_TP_CTRL_ABORT      255U 
typedef struct {
    uint8_t  priority; 
    uint8_t  dp;       
    uint8_t  pf;       
    uint8_t  ps;       
    uint8_t  sa;       
    uint8_t  da;       
    uint32_t pgn;      
    bool     is_pdu1;  
} SYN_J1939_Header;

typedef struct {
    uint32_t identity_number;   
    uint16_t manufacturer_code; 
    uint8_t  ecu_instance;      
    uint8_t  function_instance; 
    uint8_t  function;          
    uint8_t  vehicle_system;    
    uint8_t  vehicle_system_inst;
    uint8_t  industry_group;    
    bool     arbitrary_addr_cap;
} SYN_J1939_Name;

typedef struct {
    uint32_t spn;              
    uint8_t  fmi;              
    uint8_t  occurrence_count; 
    uint8_t  conversion_method;
} SYN_J1939_DTC;

typedef enum {
    SYN_J1939_STATE_UNCLAIMED = 0,
    SYN_J1939_STATE_CLAIMING  = 1,
    SYN_J1939_STATE_CLAIMED   = 2,
    SYN_J1939_STATE_CANNOT_CLAIM = 3
} SYN_J1939_AddressState;

typedef struct {
    bool     active;           
    bool     is_bam;           
    uint8_t  sa;               
    uint8_t  da;               
    uint32_t pgn;              
    uint16_t total_bytes;      
    uint8_t  total_packets;    
    uint8_t  next_seq;         
    uint8_t  received_packets; 
    uint8_t  data[1785];       
} SYN_J1939_TpRxSession;

typedef struct {
    uint8_t               sa;        
    SYN_J1939_Name        name;      
    SYN_J1939_AddressState state;     
    SYN_J1939_TpRxSession tp_rx;     
} SYN_J1939_Node;

uint32_t syn_j1939_id_pack(uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da);

SYN_Status syn_j1939_id_unpack(uint32_t can_id, SYN_J1939_Header *header);

uint32_t syn_j1939_calc_pgn(uint8_t dp, uint8_t pf, uint8_t ps);

void syn_j1939_name_encode(const SYN_J1939_Name *name, uint8_t buf[8]);

void syn_j1939_name_decode(const uint8_t buf[8], SYN_J1939_Name *name);

SYN_Status syn_j1939_node_init(SYN_J1939_Node *node, uint8_t sa, const SYN_J1939_Name *name);

SYN_Status syn_j1939_build_address_claim(const SYN_J1939_Node *node, SYN_CAN_Frame *frame);

SYN_Status syn_j1939_build_request(uint8_t sa, uint8_t da, uint32_t requested_pgn, SYN_CAN_Frame *frame);

SYN_Status syn_j1939_build_tp_bam(uint8_t sa, uint32_t pgn, uint16_t total_bytes, SYN_CAN_Frame *frame);

SYN_Status syn_j1939_build_tp_dt(uint8_t sa, uint8_t sequence, const uint8_t *payload, size_t payload_len, SYN_CAN_Frame *frame);

size_t syn_j1939_encode_dm1(uint8_t *buf, size_t buf_size, const SYN_J1939_DTC *dtc_list, size_t dtc_count, uint8_t mil_lamp_status);

SYN_Status syn_j1939_process_frame(SYN_J1939_Node *node,
                                   const SYN_CAN_Frame *frame,
                                   uint32_t *out_pgn,
                                   const uint8_t **out_data,
                                   size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_J1939_H */
```


