

# File syn\_ble\_hci.c

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_hci.c**](syn__ble__hci_8c.md)

[Go to the documentation of this file](syn__ble__hci_8c.md)


```C++

#include "syn_ble_hci.h"

#include <string.h>

#define SYN_BLE_HCI_RX_STATE_TYPE 0U    
#define SYN_BLE_HCI_RX_STATE_HDR 1U     
#define SYN_BLE_HCI_RX_STATE_PAYLOAD 2U 
SYN_Status syn_ble_hci_init(SYN_BLE_HCI *hci, const SYN_BLE_HCI_Config *cfg)
{
    if (hci == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(hci, 0, sizeof(*hci));
    if (cfg != NULL) {
        hci->cfg = *cfg;
    }
    hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
    return SYN_OK;
}

SYN_Status syn_ble_hci_encode_command(uint16_t opcode, const uint8_t *params, uint8_t param_len,
                                      uint8_t *tx_buf, uint16_t *tx_len)
{
    if (tx_buf == NULL || tx_len == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (params == NULL && param_len > 0U) {
        return SYN_INVALID_PARAM;
    }

    tx_buf[0] = SYN_BLE_HCI_PKT_CMD;
    tx_buf[1] = (uint8_t)(opcode & 0xFFU);
    tx_buf[2] = (uint8_t)((opcode >> 8U) & 0xFFU);
    tx_buf[3] = param_len;

    if (param_len > 0U) {
        memcpy(&tx_buf[4], params, param_len);
    }

    *tx_len = (uint16_t)(4U + param_len);
    return SYN_OK;
}

SYN_Status syn_ble_hci_encode_acl(uint16_t conn_handle, uint8_t pb_flags, uint8_t bc_flags,
                                  const uint8_t *data, uint16_t len, uint8_t *tx_buf,
                                  uint16_t *tx_len)
{
    if (tx_buf == NULL || tx_len == NULL || data == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint16_t handle_flags = (uint16_t)((conn_handle & 0x0FFFU) | ((pb_flags & 0x03U) << 12U) |
                                       ((bc_flags & 0x03U) << 14U));

    tx_buf[0] = SYN_BLE_HCI_PKT_ACL;
    tx_buf[1] = (uint8_t)(handle_flags & 0xFFU);
    tx_buf[2] = (uint8_t)((handle_flags >> 8U) & 0xFFU);
    tx_buf[3] = (uint8_t)(len & 0xFFU);
    tx_buf[4] = (uint8_t)((len >> 8U) & 0xFFU);

    memcpy(&tx_buf[5], data, len);
    *tx_len = (uint16_t)(5U + len);
    return SYN_OK;
}

static void parse_and_dispatch(SYN_BLE_HCI *hci)
{
    if (hci->rx_pkt_type == SYN_BLE_HCI_PKT_EVT) {
        uint8_t evt_code = hci->rx_buf[0];
        uint8_t param_len = hci->rx_buf[1];
        const uint8_t *payload = &hci->rx_buf[2];

        if (evt_code == SYN_BLE_HCI_EVT_CMD_COMPLETE && param_len >= 3U) {
            hci->last_cmd_opcode = (uint16_t)(payload[1] | (payload[2] << 8U));
            hci->last_cmd_status = (param_len >= 4U) ? payload[3] : 0U;
            hci->cmd_pending = false;
        } else if (evt_code == SYN_BLE_HCI_EVT_CMD_STATUS && param_len >= 4U) {
            hci->last_cmd_status = payload[0];
            hci->last_cmd_opcode = (uint16_t)(payload[2] | (payload[3] << 8U));
            hci->cmd_pending = false;
        }

        if (hci->cfg.evt_cb != NULL) {
            hci->cfg.evt_cb(hci, evt_code, payload, param_len, hci->cfg.user_data);
        }
    } else if (hci->rx_pkt_type == SYN_BLE_HCI_PKT_ACL) {
        uint16_t handle_flags = (uint16_t)(hci->rx_buf[0] | (hci->rx_buf[1] << 8U));
        uint16_t conn_handle = handle_flags & 0x0FFFU;
        uint8_t pb_bc_flags = (uint8_t)((handle_flags >> 12U) & 0x0FU);
        uint16_t data_len = (uint16_t)(hci->rx_buf[2] | (hci->rx_buf[3] << 8U));
        const uint8_t *data = &hci->rx_buf[4];

        if (hci->cfg.acl_cb != NULL) {
            hci->cfg.acl_cb(hci, conn_handle, pb_bc_flags, data, data_len, hci->cfg.user_data);
        }
    }
}

SYN_Status syn_ble_hci_rx_byte(SYN_BLE_HCI *hci, uint8_t byte)
{
    if (hci == NULL) {
        return SYN_INVALID_PARAM;
    }

    switch (hci->rx_state) {
    case SYN_BLE_HCI_RX_STATE_TYPE:
        if (byte == SYN_BLE_HCI_PKT_EVT || byte == SYN_BLE_HCI_PKT_ACL) {
            hci->rx_pkt_type = byte;
            hci->rx_idx = 0;
            hci->rx_target_len = (byte == SYN_BLE_HCI_PKT_EVT) ? 2U : 4U;
            hci->rx_state = SYN_BLE_HCI_RX_STATE_HDR;
        }
        break;

    case SYN_BLE_HCI_RX_STATE_HDR:
        if (hci->rx_idx < sizeof(hci->rx_buf)) {
            hci->rx_buf[hci->rx_idx++] = byte;
            if (hci->rx_idx >= hci->rx_target_len) {
                if (hci->rx_pkt_type == SYN_BLE_HCI_PKT_EVT) {
                    uint8_t param_len = hci->rx_buf[1];
                    hci->rx_target_len += param_len;
                } else if (hci->rx_pkt_type == SYN_BLE_HCI_PKT_ACL) {
                    uint16_t data_len = (uint16_t)(hci->rx_buf[2] | (hci->rx_buf[3] << 8U));
                    hci->rx_target_len += data_len;
                }

                if (hci->rx_target_len > sizeof(hci->rx_buf)) {
                    hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
                    return SYN_ERROR;
                }

                if (hci->rx_idx >= hci->rx_target_len) {
                    parse_and_dispatch(hci);
                    hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
                } else {
                    hci->rx_state = SYN_BLE_HCI_RX_STATE_PAYLOAD;
                }
            }
        } else {
            hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
            return SYN_ERROR;
        }
        break;

    case SYN_BLE_HCI_RX_STATE_PAYLOAD:
        if (hci->rx_idx < sizeof(hci->rx_buf)) {
            hci->rx_buf[hci->rx_idx++] = byte;
            if (hci->rx_idx >= hci->rx_target_len) {
                parse_and_dispatch(hci);
                hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
            }
        } else {
            hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
            return SYN_ERROR;
        }
        break;

    default:
        hci->rx_state = SYN_BLE_HCI_RX_STATE_TYPE;
        break;
    }

    return SYN_OK;
}

SYN_Status syn_ble_hci_rx_buf(SYN_BLE_HCI *hci, const uint8_t *buf, uint16_t len)
{
    if (hci == NULL || (buf == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    for (uint16_t i = 0U; i < len; i++) {
        SYN_Status st = syn_ble_hci_rx_byte(hci, buf[i]);
        if (st != SYN_OK) {
            return st;
        }
    }
    return SYN_OK;
}
```


