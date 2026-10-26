

# File syn\_ble\_att.h

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_att.h**](syn__ble__att_8h.md)

[Go to the documentation of this file](syn__ble__att_8h.md)


```C++

#ifndef SYN_BLE_ATT_H
#define SYN_BLE_ATT_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLE_ATT_OP_ERROR_RSP 0x01U              
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ 0x02U       
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP 0x03U       
#define SYN_BLE_ATT_OP_FIND_INFO_REQ 0x04U          
#define SYN_BLE_ATT_OP_FIND_INFO_RSP 0x05U          
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_REQ 0x06U 
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_RSP 0x07U 
#define SYN_BLE_ATT_OP_READ_BY_TYPE_REQ 0x08U       
#define SYN_BLE_ATT_OP_READ_BY_TYPE_RSP 0x09U       
#define SYN_BLE_ATT_OP_READ_REQ 0x0AU               
#define SYN_BLE_ATT_OP_READ_RSP 0x0BU               
#define SYN_BLE_ATT_OP_READ_BLOB_REQ 0x0CU          
#define SYN_BLE_ATT_OP_READ_BLOB_RSP 0x0DU          
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ 0x10U 
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP 0x11U 
#define SYN_BLE_ATT_OP_WRITE_REQ 0x12U              
#define SYN_BLE_ATT_OP_WRITE_RSP 0x13U              
#define SYN_BLE_ATT_OP_WRITE_CMD 0x52U              
#define SYN_BLE_ATT_OP_HANDLE_VAL_NOTIF 0x1BU       
#define SYN_BLE_ATT_OP_HANDLE_VAL_IND 0x1DU         
#define SYN_BLE_ATT_OP_HANDLE_VAL_CONF 0x1EU        
#define SYN_BLE_ATT_ERR_INVALID_HANDLE 0x01U         
#define SYN_BLE_ATT_ERR_READ_NOT_PERMITTED 0x02U     
#define SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED 0x03U    
#define SYN_BLE_ATT_ERR_INVALID_PDU 0x04U            
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTH 0x05U      
#define SYN_BLE_ATT_ERR_REQ_NOT_SUPPORTED 0x06U      
#define SYN_BLE_ATT_ERR_INVALID_OFFSET 0x07U         
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTHOR 0x08U    
#define SYN_BLE_ATT_ERR_PREPARE_QUEUE_FULL 0x09U     
#define SYN_BLE_ATT_ERR_ATTR_NOT_FOUND 0x0AU         
#define SYN_BLE_ATT_ERR_ATTR_NOT_LONG 0x0BU          
#define SYN_BLE_ATT_ERR_INSUFFICIENT_KEY_SIZE 0x0CU  
#define SYN_BLE_ATT_ERR_INVALID_ATTR_LEN 0x0DU       
#define SYN_BLE_ATT_ERR_UNLIKELY_ERROR 0x0EU         
#define SYN_BLE_ATT_ERR_INSUFFICIENT_ENCRYPT 0x0FU   
#define SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE 0x10U 
#define SYN_BLE_ATT_ERR_INSUFFICIENT_RESOURCES 0x11U 
#ifndef SYN_BLE_ATT_MTU_DEFAULT
#define SYN_BLE_ATT_MTU_DEFAULT 23U 
#endif

SYN_Status syn_ble_att_encode_error_rsp(uint8_t req_opcode, uint16_t handle, uint8_t err_code,
                                        uint8_t *buf, uint16_t *len);

SYN_Status syn_ble_att_encode_mtu_rsp(uint16_t server_mtu, uint8_t *buf, uint16_t *len);

SYN_Status syn_ble_att_encode_read_rsp(const uint8_t *val, uint16_t val_len, uint8_t *buf,
                                       uint16_t *len);

SYN_Status syn_ble_att_encode_write_rsp(uint8_t *buf, uint16_t *len);

SYN_Status syn_ble_att_encode_notification(uint16_t handle, const uint8_t *val, uint16_t val_len,
                                           uint8_t *buf, uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_ATT_H */
```


