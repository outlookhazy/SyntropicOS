/**
 * @file syn_ble_gap.c
 * @brief Zero-Heap Generic Access Profile (GAP) Layer Implementation.
 */

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
    if (gap == NULL || tx_buf == NULL || tx_len == NULL || len > 31U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t params[32];
    params[0] = len;
    memset(&params[1], 0, 31);
    if (data != NULL && len > 0U) {
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

/**
 * @brief Validate and dispatch LE Advertising Report subevent.
 * @param gap GAP context handle.
 * @param payload LE Meta event payload.
 * @param len Total LE Meta event payload length.
 * @return SYN_OK on success, SYN_ERROR on malformed report length.
 */
static SYN_Status gap_process_adv_report(SYN_BLE_GAP *gap, const uint8_t *payload, uint8_t len)
{
    if (len < 11U) {
        return SYN_ERROR;
    }

    uint8_t num_reports = payload[1];
    if (num_reports == 0U) {
        return SYN_OK;
    }

    uint8_t data_len = payload[10];
    if (data_len > 31U || (11U + (size_t)data_len) > (size_t)len) {
        return SYN_ERROR;
    }

    if (gap->evt_cb != NULL) {
        SYN_BLE_GAP_AdvReport report;
        memset(&report, 0, sizeof(report));
        report.evt_type = payload[2];
        report.addr_type = payload[3];
        memcpy(report.addr, &payload[4], 6U);
        report.data_len = data_len;
        report.data = (data_len > 0U) ? &payload[11] : NULL;
        if ((11U + (size_t)data_len) < (size_t)len) {
            report.rssi = (int8_t)payload[11U + (size_t)data_len];
        }
        gap->evt_cb(gap, SYN_BLE_GAP_EVT_ADV_REPORT, &report, gap->user_data);
    }

    return SYN_OK;
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
            uint8_t status = payload[1];
            if (status == 0x00U && gap->evt_cb != NULL) {
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
        } else if (subevt == SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT) {
            return gap_process_adv_report(gap, payload, len);
        }
    }

    return SYN_OK;
}
