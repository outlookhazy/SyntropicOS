

# File syn\_ble\_gap.c

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gap.c**](syn__ble__gap_8c.md)

[Go to the documentation of this file](syn__ble__gap_8c.md)


```C++


#include "syn_ble_gap.h"

#include <string.h>

SYN_Status syn_ble_gap_init(SYN_BLE_GAP *gap, SYN_BLE_HCI *hci, SYN_BLE_GAP_EventCb evt_cb,
                            void *user_data)
{
    if (gap == NULL || hci == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(gap, 0, sizeof(*gap));
    gap->hci = hci;
    gap->evt_cb = evt_cb;
    gap->user_data = user_data;
    return SYN_OK;
}

SYN_Status syn_ble_gap_set_adv_data(SYN_BLE_GAP *gap, const uint8_t *data, uint8_t len,
                                    uint8_t *tx_buf, uint16_t *tx_len)
{
    if (gap == NULL || tx_buf == NULL || tx_len == NULL || len > 31U ||
        (data == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t params[32];
    params[0] = len;
    memset(&params[1], 0, 31);
    if (len > 0U) {
        memcpy(&params[1], data, len);
    }

    return syn_ble_hci_encode_command(SYN_BLE_HCI_OP_LE_SET_ADV_DATA, params, 32U, tx_buf, tx_len);
}

SYN_Status syn_ble_gap_set_adv_enable(SYN_BLE_GAP *gap, bool enable, uint8_t *tx_buf,
                                      uint16_t *tx_len)
{
    if (gap == NULL || tx_buf == NULL || tx_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t param = enable ? 0x01U : 0x00U;
    gap->advertising = enable;
    return syn_ble_hci_encode_command(SYN_BLE_HCI_OP_LE_SET_ADV_ENABLE, &param, 1U, tx_buf, tx_len);
}

static bool gap_adv_reports_valid(const uint8_t *payload, size_t len, size_t num)
{
    if (num > 25U) {
        return false;
    }

    size_t off = 2U;
    for (size_t i = 0U; i < num; i++) {
        if ((off + 9U) > len) {
            return false;
        }
        uint8_t dlen = payload[off + 8U];
        if ((dlen > 31U) || ((off + 9U + (size_t)dlen) > len)) {
            return false;
        }
        off += 9U + (size_t)dlen;
        if (off < len) {
            off += 1U;
        } else if (num > 1U) {
            return false;
        }
    }
    return true;
}

static SYN_Status gap_process_adv_report(SYN_BLE_GAP *gap, const uint8_t *payload, uint8_t len)
{
    if (len < 2U) {
        return SYN_ERROR;
    }

    size_t num = (size_t)payload[1];
    if (num == 0U) {
        return SYN_OK;
    }
    if (!gap_adv_reports_valid(payload, (size_t)len, num)) {
        return SYN_ERROR;
    }

    if (gap->evt_cb != NULL) {
        size_t off = 2U;
        for (size_t i = 0U; i < num; i++) {
            uint8_t dlen = payload[off + 8U];
            SYN_BLE_GAP_AdvReport report;
            memset(&report, 0, sizeof(report));
            report.evt_type = payload[off];
            report.addr_type = payload[off + 1U];
            memcpy(report.addr, &payload[off + 2U], 6U);
            report.data_len = dlen;
            report.data = (dlen > 0U) ? &payload[off + 9U] : NULL;
            off += 9U + (size_t)dlen;
            if (off < (size_t)len) {
                report.rssi = (int8_t)payload[off];
                off += 1U;
            }
            gap->evt_cb(gap, SYN_BLE_GAP_EVT_ADV_REPORT, &report, gap->user_data);
        }
    }

    return SYN_OK;
}

static void gap_process_conn_complete(SYN_BLE_GAP *gap, const uint8_t *payload)
{
    if (payload[1] != 0x00U) {
        return;
    }
    if (payload[4] == 0x01U) {
        gap->advertising = false;
    }
    if (gap->evt_cb != NULL) {
        SYN_BLE_GAP_ConnInfo info;
        memset(&info, 0, sizeof(info));
        info.conn_handle =
            (uint16_t)((uint16_t)payload[2] | (uint16_t)((uint16_t)payload[3] << 8U));
        info.peer_addr_type = payload[5];
        memcpy(info.peer_addr, &payload[6], 6U);
        info.conn_interval =
            (uint16_t)((uint16_t)payload[12] | (uint16_t)((uint16_t)payload[13] << 8U));
        info.conn_latency =
            (uint16_t)((uint16_t)payload[14] | (uint16_t)((uint16_t)payload[15] << 8U));
        info.supervision_timeout =
            (uint16_t)((uint16_t)payload[16] | (uint16_t)((uint16_t)payload[17] << 8U));
        gap->evt_cb(gap, SYN_BLE_GAP_EVT_CONNECTED, &info, gap->user_data);
    }
}

SYN_Status syn_ble_gap_process_hci_evt(SYN_BLE_GAP *gap, uint8_t evt_code, const uint8_t *payload,
                                       uint8_t len)
{
    if (gap == NULL || payload == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (evt_code == SYN_BLE_HCI_EVT_DISCONN_COMPLETE && len >= 4U) {
        uint8_t status = payload[0];
        uint16_t conn_handle =
            (uint16_t)((uint16_t)payload[1] | (uint16_t)((uint16_t)payload[2] << 8U));

        if (status == 0x00U && gap->evt_cb != NULL) {
            SYN_BLE_GAP_ConnInfo info;
            memset(&info, 0, sizeof(info));
            info.conn_handle = conn_handle;
            gap->evt_cb(gap, SYN_BLE_GAP_EVT_DISCONNECTED, &info, gap->user_data);
        }
    } else if (evt_code == SYN_BLE_HCI_EVT_LE_META && len >= 1U) {
        uint8_t subevt = payload[0];
        if (subevt == SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE && len >= 19U) {
            gap_process_conn_complete(gap, payload);
        } else if (subevt == SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT) {
            return gap_process_adv_report(gap, payload, len);
        }
    }

    return SYN_OK;
}
```


