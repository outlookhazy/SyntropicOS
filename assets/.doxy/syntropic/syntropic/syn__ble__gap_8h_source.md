

# File syn\_ble\_gap.h

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gap.h**](syn__ble__gap_8h.md)

[Go to the documentation of this file](syn__ble__gap_8h.md)


```C++

#ifndef SYN_BLE_GAP_H
#define SYN_BLE_GAP_H

#include "../common/syn_defs.h"
#include "syn_ble_hci.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLE_ADV_IND 0x00U         
#define SYN_BLE_ADV_DIRECT_IND 0x01U  
#define SYN_BLE_ADV_SCAN_IND 0x02U    
#define SYN_BLE_ADV_NONCONN_IND 0x03U 
typedef enum {
    SYN_BLE_GAP_EVT_CONNECTED = 0, 
    SYN_BLE_GAP_EVT_DISCONNECTED,  
    SYN_BLE_GAP_EVT_ADV_REPORT     
} SYN_BLE_GAP_EventType;

typedef struct {
    uint16_t conn_handle;         
    uint8_t peer_addr_type;       
    uint8_t peer_addr[6];         
    uint16_t conn_interval;       
    uint16_t conn_latency;        
    uint16_t supervision_timeout; 
} SYN_BLE_GAP_ConnInfo;

typedef struct {
    uint8_t evt_type;    
    uint8_t addr_type;   
    uint8_t addr[6];     
    const uint8_t *data; 
    uint8_t data_len;    
    int8_t rssi;         
} SYN_BLE_GAP_AdvReport;

typedef struct SYN_BLE_GAP_s SYN_BLE_GAP;

typedef void (*SYN_BLE_GAP_EventCb)(SYN_BLE_GAP *gap, SYN_BLE_GAP_EventType evt_type,
                                    const void *evt_data, void *user_data);

typedef struct {
    uint16_t interval_min;  
    uint16_t interval_max;  
    uint8_t type;           
    uint8_t own_addr_type;  
    uint8_t peer_addr_type; 
    uint8_t peer_addr[6];   
    uint8_t channel_map;    
    uint8_t filter_policy;  
} SYN_BLE_GAP_AdvParams;

struct SYN_BLE_GAP_s {
    SYN_BLE_HCI *hci;           
    SYN_BLE_GAP_EventCb evt_cb; 
    void *user_data;            
    bool advertising;           
    bool scanning;              
};

SYN_Status syn_ble_gap_init(SYN_BLE_GAP *gap, SYN_BLE_HCI *hci, SYN_BLE_GAP_EventCb evt_cb,
                            void *user_data);

SYN_Status syn_ble_gap_set_adv_data(SYN_BLE_GAP *gap, const uint8_t *data, uint8_t len,
                                    uint8_t *tx_buf, uint16_t *tx_len);

SYN_Status syn_ble_gap_set_adv_enable(SYN_BLE_GAP *gap, bool enable, uint8_t *tx_buf,
                                      uint16_t *tx_len);

SYN_Status syn_ble_gap_process_hci_evt(SYN_BLE_GAP *gap, uint8_t evt_code, const uint8_t *payload,
                                       uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_GAP_H */
```


