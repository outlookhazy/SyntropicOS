

# File syn\_devicenet.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_devicenet.h**](syn__devicenet_8h.md)

[Go to the documentation of this file](syn__devicenet_8h.md)


```C++

#ifndef SYN_DEVICENET_H
#define SYN_DEVICENET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DEVICENET_MAX_MAC_ID 63U           
#define SYN_DEVICENET_MAX_PAYLOAD 8U           
#define SYN_DEVICENET_DUP_MAC_TIMEOUT_MS 1000U 
typedef enum {
    SYN_DEVICENET_BAUD_125K = 0U, 
    SYN_DEVICENET_BAUD_250K = 1U, 
    SYN_DEVICENET_BAUD_500K = 2U  
} SYN_DeviceNet_Baud;

typedef enum {
    SYN_DEVICENET_STATE_OFFLINE = 0U,       
    SYN_DEVICENET_STATE_DUP_MAC_CHECK = 1U, 
    SYN_DEVICENET_STATE_ONLINE = 2U,        
    SYN_DEVICENET_STATE_BUS_OFF = 3U        
} SYN_DeviceNet_State;

typedef struct {
    uint16_t vendor_id;         
    uint16_t device_type;       
    uint16_t product_code;      
    uint8_t rev_major;          
    uint8_t rev_minor;          
    uint16_t status;            
    uint32_t serial_number;     
    char product_name[32];      
    bool quick_connect_enabled; 
} SYN_CIP_Identity;

typedef struct {
    uint8_t mac_id;               
    SYN_DeviceNet_Baud baud_rate; 
    uint8_t bus_off_action;       
    uint8_t allocation_choice;    
    uint8_t master_mac_id;        
} SYN_CIP_DeviceNet;

typedef struct {
    uint8_t *input_buf;  
    uint8_t input_len;   
    uint8_t *output_buf; 
    uint8_t output_len;  
} SYN_CIP_Assembly;

typedef struct {
    SYN_DeviceNet_State state;       
    SYN_CIP_Identity identity;       
    SYN_CIP_DeviceNet devicenet_obj; 
    SYN_CIP_Assembly assembly;       
    uint32_t dup_mac_timer_ms;       
    bool dup_mac_failed;             
    bool explicit_connected;         
    bool polled_connected;           
} SYN_DeviceNet_Node;

bool syn_devicenet_init(SYN_DeviceNet_Node *node, uint8_t mac_id, SYN_DeviceNet_Baud baud);

void syn_devicenet_set_serial_number(SYN_DeviceNet_Node *node, uint32_t serial_number);

bool syn_devicenet_set_assembly(SYN_DeviceNet_Node *node, uint8_t *in_buf, uint8_t in_len,
                                uint8_t *out_buf, uint8_t out_len);

bool syn_devicenet_set_quickconnect(SYN_DeviceNet_Node *node, bool enable);

bool syn_devicenet_on_can_rx(SYN_DeviceNet_Node *node, uint32_t can_id, const uint8_t *data,
                             uint8_t len, uint32_t *tx_can_id, uint8_t *tx_data, uint8_t *tx_len);

void syn_devicenet_poll(SYN_DeviceNet_Node *node, uint32_t dt_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DEVICENET_H */
```


