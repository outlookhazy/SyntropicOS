

# File syn\_doip.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_doip.h**](syn__doip_8h.md)

[Go to the documentation of this file](syn__doip_8h.md)


```C++

#ifndef SYNTROPIC_DOIP_H
#define SYNTROPIC_DOIP_H

#include "syntropic/proto/syn_uds.h"
#include "syntropic/syntropic.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DOIP_PROTOCOL_VERSION 0x02U         
#define SYN_DOIP_INVERSE_PROTOCOL_VERSION 0xFDU 
#define SYN_DOIP_HEADER_SIZE 8U                 
#define SYN_DOIP_DEFAULT_PORT 13400U            
#define SYN_DOIP_TYPE_GENERIC_NACK 0x0000U            
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ 0x0001U          
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID 0x0002U      
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_VIN 0x0003U      
#define SYN_DOIP_TYPE_VEHICLE_ANNOUNCEMENT 0x0004U    
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_REQ 0x0005U  
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_RESP 0x0006U 
#define SYN_DOIP_TYPE_ALIVE_CHECK_REQ 0x0007U         
#define SYN_DOIP_TYPE_ALIVE_CHECK_RESP 0x0008U        
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG 0x8001U          
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_ACK 0x8002U      
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_NACK 0x8003U     
#define SYN_DOIP_NACK_INCORRECT_PATTERN 0x00U      
#define SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE 0x01U   
#define SYN_DOIP_NACK_MESSAGE_TOO_LARGE 0x02U      
#define SYN_DOIP_NACK_OUT_OF_MEMORY 0x03U          
#define SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH 0x04U 
#define SYN_DOIP_ROUTING_SUCCESS 0x00U               
#define SYN_DOIP_ROUTING_DENIED_UNKNOWN_TESTER 0x02U 
#define SYN_DOIP_ROUTING_DENIED_DIFFERENT_PORT 0x04U 
typedef struct {
    uint8_t protocol_version;         
    uint8_t inverse_protocol_version; 
    uint16_t payload_type;            
    uint32_t payload_length;          
} SYN_DoIP_Header;

typedef struct {
    uint16_t logical_address;       
    uint16_t active_tester_address; 
    bool routing_activated;         
    uint8_t vin[17];                
    uint8_t eid[6];                 
    uint8_t gid[6];                 
} SYN_DoIP_Server;

bool syn_doip_init(SYN_DoIP_Server *server, uint16_t logical_address);

bool syn_doip_set_identifiers(SYN_DoIP_Server *server, const uint8_t vin[17], const uint8_t eid[6],
                              const uint8_t gid[6]);

bool syn_doip_parse_header(const uint8_t *buf, uint16_t len, SYN_DoIP_Header *hdr);

uint16_t syn_doip_encode_header(const SYN_DoIP_Header *hdr, uint8_t *buf, uint16_t max_len);

bool syn_doip_process_msg(SYN_DoIP_Server *server, SYN_UDS_Server *uds, const uint8_t *rx_buf,
                          uint16_t rx_len, uint8_t *tx_buf, uint16_t max_tx_len, uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYNTROPIC_DOIP_H */
```


