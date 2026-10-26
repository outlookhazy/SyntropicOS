

# File syn\_ble\_gatt.h

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gatt.h**](syn__ble__gatt_8h.md)

[Go to the documentation of this file](syn__ble__gatt_8h.md)


```C++

#ifndef SYN_BLE_GATT_H
#define SYN_BLE_GATT_H

#include "../common/syn_defs.h"
#include "syn_ble_att.h"
#include "syn_ble_l2cap.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLE_UUID_PRIMARY_SERVICE 0x2800U   
#define SYN_BLE_UUID_SECONDARY_SERVICE 0x2801U 
#define SYN_BLE_UUID_CHARACTERISTIC 0x2803U    
#define SYN_BLE_UUID_CCCD 0x2902U 
#define SYN_BLE_PROP_READ 0x02U      
#define SYN_BLE_PROP_WRITE_CMD 0x04U 
#define SYN_BLE_PROP_WRITE 0x08U     
#define SYN_BLE_PROP_NOTIFY 0x10U    
#define SYN_BLE_PROP_INDICATE 0x20U  
#ifndef SYN_BLE_MAX_ATTRIBUTES
#define SYN_BLE_MAX_ATTRIBUTES 32U 
#endif

typedef struct SYN_BLE_GATT_s SYN_BLE_GATT;

typedef SYN_Status (*SYN_BLE_GATT_ReadCb)(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                          uint16_t attr_handle, uint8_t *val, uint16_t *val_len,
                                          void *user_data);

typedef SYN_Status (*SYN_BLE_GATT_WriteCb)(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                           uint16_t attr_handle, const uint8_t *val,
                                           uint16_t val_len, void *user_data);

typedef struct {
    uint16_t handle;               
    uint16_t uuid16;               
    uint8_t permissions;           
    SYN_BLE_GATT_ReadCb read_cb;   
    SYN_BLE_GATT_WriteCb write_cb; 
    const uint8_t *static_val;     
    uint16_t static_val_len;       
} SYN_BLE_GATT_Attr;

struct SYN_BLE_GATT_s {
    const SYN_BLE_GATT_Attr *table;              
    uint16_t table_len;                          
    uint16_t cccd_vals[SYN_BLE_MAX_CONNECTIONS]; 
    void *user_data;                             
};

SYN_Status syn_ble_gatt_init(SYN_BLE_GATT *gatt, const SYN_BLE_GATT_Attr *table, uint16_t table_len,
                             void *user_data);

SYN_Status syn_ble_gatt_process_att_pdu(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                        uint16_t *resp_len);

SYN_Status syn_ble_gatt_notify(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                               const uint8_t *val, uint16_t val_len, uint8_t *tx_buf,
                               uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_GATT_H */
```


