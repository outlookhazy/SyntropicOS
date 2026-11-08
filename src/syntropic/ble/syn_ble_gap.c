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

/**
 * @brief Validate and dispatch LE Advertising Report subevent.
 * @param gap GAP context handle.
 * @param payload LE Meta event payload.
 * @param len Total LE Meta event payload length.
 * @return SYN_OK on success, SYN_ERROR on malformed report length.
 */
static SYN_Status gap_process_adv_report(SYN_BLE_GAP *gap, const uint8_t *payload, uint8_t len)
{
    if (len < 2U) {
        return SYN_ERROR;
    }

    size_t num = (size_t)payload[1];
    if (num == 0U) {
        return SYN_OK;
    }

    size_t data_base = 2U + (9U * num);
    if (num > 25U || (size_t)len < data_base) {
        return SYN_ERROR;
    }

    size_t dlen_base = 2U + (8U * num);
    size_t total_data = 0U;
    for (size_t i = 0U; i < num; i++) {
        if (payload[dlen_base + i] > 31U) {
            return SYN_ERROR;
        }
        total_data += (size_t)payload[dlen_base + i];
    }

    size_t rssi_base = data_base + total_data;
    if (rssi_base > (size_t)len) {
        return SYN_ERROR;
    }

    if (gap->evt_cb != NULL) {
        bool has_rssi = (rssi_base + num) <= (size_t)len;
        size_t data_off = data_base;
        for (size_t i = 0U; i < num; i++) {
            uint8_t dlen = payload[dlen_base + i];
            SYN_BLE_GAP_AdvReport report;
            memset(&report, 0, sizeof(report));
            report.evt_type = payload[2U + i];
            report.addr_type = payload[2U + num + i];
            memcpy(report.addr, &payload[2U + (2U * num) + (6U * i)], 6U);
            report.data_len = dlen;
            report.data = (dlen > 0U) ? &payload[data_off] : NULL;
            report.rssi = has_rssi ? (int8_t)payload[rssi_base + i] : 0;
            data_off += (size_t)dlen;
            gap->evt_cb(gap, SYN_BLE_GAP_EVT_ADV_REPORT, &report, gap->user_data);
        }
    }

    return SYN_OK;
}

/**
 * @brief Handle LE Connection Complete subevent, clearing advertising state on Peripheral role.
 * @param gap     GAP context handle.
 * @param payload 19-byte LE Connection Complete Meta event payload.
 */
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
