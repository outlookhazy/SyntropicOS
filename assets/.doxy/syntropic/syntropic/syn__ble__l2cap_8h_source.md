

# File syn\_ble\_l2cap.h

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_l2cap.h**](syn__ble__l2cap_8h.md)

[Go to the documentation of this file](syn__ble__l2cap_8h.md)


```C++

#ifndef SYN_BLE_L2CAP_H
#define SYN_BLE_L2CAP_H

#include "../common/syn_defs.h"
#include "syn_ble_hci.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLE_L2CAP_CID_ATT 0x0004U       
#define SYN_BLE_L2CAP_CID_SIGNALING 0x0005U 
#define SYN_BLE_L2CAP_CID_SMP 0x0006U       
#ifndef SYN_BLE_MAX_CONNECTIONS
#define SYN_BLE_MAX_CONNECTIONS 2U 
#endif

#ifndef SYN_BLE_L2CAP_BUF_SIZE
#define SYN_BLE_L2CAP_BUF_SIZE 256U 
#endif

typedef struct SYN_BLE_L2CAP_s SYN_BLE_L2CAP;

typedef void (*SYN_BLE_L2CAP_RxCb)(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle, uint16_t cid,
                                   const uint8_t *payload, uint16_t len, void *user_data);

typedef struct {
    uint16_t conn_handle;                   
    uint8_t rx_buf[SYN_BLE_L2CAP_BUF_SIZE]; 
    uint16_t rx_len;                        
    uint16_t expected_len;                  
    uint16_t cid;                           
    bool active;                            
    bool reassembling;                      
} SYN_BLE_L2CAP_Connection;

struct SYN_BLE_L2CAP_s {
    SYN_BLE_L2CAP_Connection conns[SYN_BLE_MAX_CONNECTIONS]; 
    SYN_BLE_L2CAP_RxCb rx_cb;                                
    void *user_data;                                         
};

SYN_Status syn_ble_l2cap_init(SYN_BLE_L2CAP *l2cap, SYN_BLE_L2CAP_RxCb rx_cb, void *user_data);

SYN_Status syn_ble_l2cap_connect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle);

SYN_Status syn_ble_l2cap_disconnect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle);

SYN_Status syn_ble_l2cap_process_acl(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle,
                                     uint8_t pb_bc_flags, const uint8_t *data, uint16_t len);

SYN_Status syn_ble_l2cap_encode_pdu(uint16_t conn_handle, uint16_t cid, const uint8_t *payload,
                                    uint16_t len, uint8_t *tx_buf, uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_L2CAP_H */
```


