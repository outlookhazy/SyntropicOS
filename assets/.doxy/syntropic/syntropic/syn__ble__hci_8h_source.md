

# File syn\_ble\_hci.h

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_hci.h**](syn__ble__hci_8h.md)

[Go to the documentation of this file](syn__ble__hci_8h.md)


```C++

#ifndef SYN_BLE_HCI_H
#define SYN_BLE_HCI_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLE_HCI_PKT_CMD 0x01U 
#define SYN_BLE_HCI_PKT_ACL 0x02U 
#define SYN_BLE_HCI_PKT_SCO 0x03U 
#define SYN_BLE_HCI_PKT_EVT 0x04U 
#define SYN_BLE_HCI_PKT_ISO 0x05U 
#define SYN_BLE_HCI_OGF_LINK_CONTROL 0x01U  
#define SYN_BLE_HCI_OGF_INFO_PARAM 0x04U    
#define SYN_BLE_HCI_OGF_STATUS_PARAM 0x05U  
#define SYN_BLE_HCI_OGF_LE_CONTROLLER 0x08U 
#define SYN_BLE_HCI_OPCODE(ogf, ocf) ((uint16_t)(((ogf) << 10) | ((ocf) & 0x03FFU)))

#define SYN_BLE_HCI_OP_LE_SET_EVENT_MASK \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0001U) 
#define SYN_BLE_HCI_OP_LE_READ_BUFFER_SIZE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0002U) 
#define SYN_BLE_HCI_OP_LE_SET_RANDOM_ADDR \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0005U) 
#define SYN_BLE_HCI_OP_LE_SET_ADV_PARAM \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0006U) 
#define SYN_BLE_HCI_OP_LE_SET_ADV_DATA \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0008U) 
#define SYN_BLE_HCI_OP_LE_SET_SCAN_RESP_DATA \
    SYN_BLE_HCI_OPCODE(0x08U, 0x0009U) 
#define SYN_BLE_HCI_OP_LE_SET_ADV_ENABLE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000AU) 
#define SYN_BLE_HCI_OP_LE_SET_SCAN_PARAM \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000BU) 
#define SYN_BLE_HCI_OP_LE_SET_SCAN_ENABLE \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000CU) 
#define SYN_BLE_HCI_OP_LE_CREATE_CONN \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000DU) 
#define SYN_BLE_HCI_OP_LE_CREATE_CONN_CANCEL \
    SYN_BLE_HCI_OPCODE(0x08U, 0x000EU) 
#define SYN_BLE_HCI_OP_DISCONNECT SYN_BLE_HCI_OPCODE(0x01U, 0x0006U) 
#define SYN_BLE_HCI_EVT_DISCONN_COMPLETE 0x05U 
#define SYN_BLE_HCI_EVT_CMD_COMPLETE 0x0EU     
#define SYN_BLE_HCI_EVT_CMD_STATUS 0x0FU       
#define SYN_BLE_HCI_EVT_LE_META 0x3EU          
#define SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE 0x01U 
#define SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT 0x02U    
#define SYN_BLE_HCI_LE_SUBEVT_CONN_UPDATE_COMPLETE \
    0x03U 
#ifndef SYN_BLE_HCI_RX_BUF_SIZE
#define SYN_BLE_HCI_RX_BUF_SIZE 256U 
#endif

#ifndef SYN_BLE_HCI_TX_BUF_SIZE
#define SYN_BLE_HCI_TX_BUF_SIZE 256U 
#endif

typedef struct SYN_BLE_HCI_s SYN_BLE_HCI;

typedef void (*SYN_BLE_HCI_EventCb)(SYN_BLE_HCI *hci, uint8_t evt_code, const uint8_t *payload,
                                    uint8_t len, void *user_data);

typedef void (*SYN_BLE_HCI_AclCb)(SYN_BLE_HCI *hci, uint16_t conn_handle, uint8_t pb_bc_flags,
                                  const uint8_t *data, uint16_t len, void *user_data);

typedef struct {
    SYN_BLE_HCI_EventCb evt_cb; 
    SYN_BLE_HCI_AclCb acl_cb;   
    void *user_data;            
} SYN_BLE_HCI_Config;

struct SYN_BLE_HCI_s {
    SYN_BLE_HCI_Config cfg;                  
    uint8_t rx_buf[SYN_BLE_HCI_RX_BUF_SIZE]; 
    uint16_t rx_idx;                         
    uint16_t rx_target_len;                  
    uint8_t rx_state;                        
    uint8_t rx_pkt_type;                     
    uint16_t last_cmd_opcode;                
    uint8_t last_cmd_status;                 
    bool cmd_pending;                        
};

SYN_Status syn_ble_hci_init(SYN_BLE_HCI *hci, const SYN_BLE_HCI_Config *cfg);

SYN_Status syn_ble_hci_encode_command(uint16_t opcode, const uint8_t *params, uint8_t param_len,
                                      uint8_t *tx_buf, uint16_t *tx_len);

SYN_Status syn_ble_hci_encode_acl(uint16_t conn_handle, uint8_t pb_flags, uint8_t bc_flags,
                                  const uint8_t *data, uint16_t len, uint8_t *tx_buf,
                                  uint16_t *tx_len);

SYN_Status syn_ble_hci_rx_byte(SYN_BLE_HCI *hci, uint8_t byte);

SYN_Status syn_ble_hci_rx_buf(SYN_BLE_HCI *hci, const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLE_HCI_H */
```


