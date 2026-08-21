

# File syn\_bacnet.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_bacnet.h**](syn__bacnet_8h.md)

[Go to the documentation of this file](syn__bacnet_8h.md)


```C++

#ifndef SYN_BACNET_H
#define SYN_BACNET_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── BACnet MS/TP Constants ─────────────────────────────────────────────── */

#define SYN_BACNET_MSTP_PREAMBLE_1 0x55U 
#define SYN_BACNET_MSTP_PREAMBLE_2 0xFFU 
#define SYN_BACNET_BROADCAST_MAC 0xFFU   
typedef enum {
    SYN_BACNET_MSTP_FRAME_TOKEN = 0x00U,
    SYN_BACNET_MSTP_FRAME_POLL_FOR_MASTER = 0x01U,
    SYN_BACNET_MSTP_FRAME_REPLY_TO_POLL_FOR_MASTER = 0x02U,
    SYN_BACNET_MSTP_FRAME_TEST_REQUEST = 0x03U,
    SYN_BACNET_MSTP_FRAME_TEST_RESPONSE = 0x04U,
    SYN_BACNET_MSTP_FRAME_DATA_NOT_EXPECTING_REPLY = 0x05U,
    SYN_BACNET_MSTP_FRAME_DATA_EXPECTING_REPLY = 0x06U,
    SYN_BACNET_MSTP_FRAME_REPLY_POSTPONED = 0x07U,
} SYN_BACnet_MSTP_FrameType;

typedef enum {
    SYN_BACNET_SERVICE_UNCONFIRMED_I_AM = 0U,
    SYN_BACNET_SERVICE_UNCONFIRMED_WHO_IS = 8U,
    SYN_BACNET_SERVICE_CONFIRMED_READ_PROPERTY = 12U,
    SYN_BACNET_SERVICE_CONFIRMED_WRITE_PROPERTY = 15U,
} SYN_BACnet_ServiceChoice;

typedef enum {
    SYN_BACNET_OBJ_ANALOG_INPUT = 0U,
    SYN_BACNET_OBJ_ANALOG_OUTPUT = 1U,
    SYN_BACNET_OBJ_BINARY_INPUT = 3U,
    SYN_BACNET_OBJ_BINARY_OUTPUT = 4U,
    SYN_BACNET_OBJ_DEVICE = 8U,
} SYN_BACnet_ObjectType;

typedef enum {
    SYN_BACNET_PROP_PRESENT_VALUE = 85U,
    SYN_BACNET_PROP_STATUS_FLAGS = 111U,
    SYN_BACNET_PROP_OBJECT_IDENTIFIER = 75U,
    SYN_BACNET_PROP_OBJECT_NAME = 77U,
    SYN_BACNET_PROP_OBJECT_TYPE = 79U,
    SYN_BACNET_PROP_VENDOR_ID = 121U,
} SYN_BACnet_PropertyID;

/* ── Data Structures ────────────────────────────────────────────────────── */

typedef struct {
    uint8_t frame_type;      
    uint8_t destination_mac; 
    uint8_t source_mac;      
    uint16_t data_len;       
    uint8_t payload[501];    
} SYN_BACnet_MSTP_Frame;

typedef struct {
    uint8_t object_type;  
    uint32_t instance_id; 
    float present_value;  
    const char *name;     
} SYN_BACnet_Object;

#ifndef SYN_BACNET_MAX_OBJECTS
#define SYN_BACNET_MAX_OBJECTS 16 
#endif

typedef struct {
    uint8_t mac_address;                               
    uint32_t device_id;                                
    uint8_t max_master;                                
    SYN_BACnet_Object objects[SYN_BACNET_MAX_OBJECTS]; 
    size_t object_count;                               
} SYN_BACnet_Node;

/* ── API Functions ──────────────────────────────────────────────────────── */

uint8_t syn_bacnet_crc8(const uint8_t *data, size_t len);

uint16_t syn_bacnet_crc16(const uint8_t *data, size_t len);

size_t syn_bacnet_mstp_encode_frame(uint8_t frame_type, uint8_t dest_mac, uint8_t src_mac,
                                    const uint8_t *payload, uint16_t payload_len, uint8_t *out_buf);

bool syn_bacnet_mstp_decode_frame(const uint8_t *buf, size_t len, SYN_BACnet_MSTP_Frame *frame);

SYN_Status syn_bacnet_node_init(SYN_BACnet_Node *node, uint8_t mac_address, uint32_t device_id);

SYN_Status syn_bacnet_add_object(SYN_BACnet_Node *node, uint8_t object_type, uint32_t instance_id,
                                 float init_value, const char *name);

SYN_Status syn_bacnet_node_process(SYN_BACnet_Node *node, const SYN_BACnet_MSTP_Frame *rx_frame,
                                   SYN_BACnet_MSTP_Frame *tx_frame, bool *has_tx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BACNET_H */
```


