/**
 * @file syn_ble_l2cap.c
 * @brief Zero-Heap BLE L2CAP Protocol Implementation.
 */

#include "syn_ble_l2cap.h"

#include <string.h>

SYN_Status syn_ble_l2cap_init(SYN_BLE_L2CAP *l2cap, SYN_BLE_L2CAP_RxCb rx_cb, void *user_data)
{
    if (l2cap == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(l2cap, 0, sizeof(*l2cap));
    l2cap->rx_cb = rx_cb;
    l2cap->user_data = user_data;
    return SYN_OK;
}

SYN_Status syn_ble_l2cap_connect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle)
{
    if (l2cap == NULL) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0U; i < SYN_BLE_MAX_CONNECTIONS; i++) {
        if (l2cap->conns[i].active && l2cap->conns[i].conn_handle == conn_handle) {
            return SYN_OK;
        }
    }

    for (size_t i = 0U; i < SYN_BLE_MAX_CONNECTIONS; i++) {
        if (!l2cap->conns[i].active) {
            memset(&l2cap->conns[i], 0, sizeof(l2cap->conns[i]));
            l2cap->conns[i].conn_handle = conn_handle;
            l2cap->conns[i].active = true;
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

SYN_Status syn_ble_l2cap_disconnect(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle)
{
    if (l2cap == NULL) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0U; i < SYN_BLE_MAX_CONNECTIONS; i++) {
        if (l2cap->conns[i].active && l2cap->conns[i].conn_handle == conn_handle) {
            memset(&l2cap->conns[i], 0, sizeof(l2cap->conns[i]));
            return SYN_OK;
        }
    }
    return SYN_OK;
}

/**
 * @brief Find or allocate connection slot for incoming ACL packet.
 * @param l2cap L2CAP context handle.
 * @param conn_handle Connection handle.
 * @return Active connection entry pointer, or NULL if pool full.
 */
static SYN_BLE_L2CAP_Connection *l2cap_get_or_alloc_conn(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle)
{
    for (size_t i = 0U; i < SYN_BLE_MAX_CONNECTIONS; i++) {
        if (l2cap->conns[i].active && l2cap->conns[i].conn_handle == conn_handle) {
            return &l2cap->conns[i];
        }
    }
    for (size_t i = 0U; i < SYN_BLE_MAX_CONNECTIONS; i++) {
        if (!l2cap->conns[i].active) {
            memset(&l2cap->conns[i], 0, sizeof(l2cap->conns[i]));
            l2cap->conns[i].conn_handle = conn_handle;
            l2cap->conns[i].active = true;
            return &l2cap->conns[i];
        }
    }
    return NULL;
}

SYN_Status syn_ble_l2cap_process_acl(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle,
                                     uint8_t pb_bc_flags, const uint8_t *data, uint16_t len)
{
    if (l2cap == NULL || data == NULL || len == 0U) {
        return SYN_INVALID_PARAM;
    }

    SYN_BLE_L2CAP_Connection *conn = l2cap_get_or_alloc_conn(l2cap, conn_handle);
    if (conn == NULL) {
        return SYN_ERROR;
    }

    uint8_t pb_flags = pb_bc_flags & 0x03U;

    if (pb_flags == 0x02U || pb_flags == 0x00U) {
        if (len < 4U) {
            conn->reassembling = false;
            return SYN_INVALID_PARAM;
        }
        uint16_t pdu_len = (uint16_t)((uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8U));
        uint16_t cid = (uint16_t)((uint16_t)data[2] | (uint16_t)((uint16_t)data[3] << 8U));
        const uint8_t *pdu_payload = &data[4];
        uint16_t payload_len = (uint16_t)(len - 4U);

        if ((size_t)pdu_len > sizeof(conn->rx_buf) || payload_len > pdu_len) {
            conn->reassembling = false;
            return SYN_ERROR;
        }

        if (payload_len == pdu_len) {
            conn->reassembling = false;
            if (l2cap->rx_cb != NULL) {
                l2cap->rx_cb(l2cap, conn_handle, cid, pdu_payload, pdu_len, l2cap->user_data);
            }
        } else {
            memcpy(conn->rx_buf, pdu_payload, payload_len);
            conn->rx_len = payload_len;
            conn->expected_len = pdu_len;
            conn->cid = cid;
            conn->reassembling = true;
        }
        return SYN_OK;
    }

    if (pb_flags == 0x01U) {
        if (!conn->reassembling) {
            return SYN_ERROR;
        }
        if ((uint32_t)conn->rx_len + (uint32_t)len > (uint32_t)conn->expected_len) {
            conn->reassembling = false;
            return SYN_ERROR;
        }
        memcpy(&conn->rx_buf[conn->rx_len], data, len);
        conn->rx_len = (uint16_t)(conn->rx_len + len);

        if (conn->rx_len == conn->expected_len) {
            conn->reassembling = false;
            if (l2cap->rx_cb != NULL) {
                l2cap->rx_cb(l2cap, conn_handle, conn->cid, conn->rx_buf, conn->expected_len,
                             l2cap->user_data);
            }
        }
        return SYN_OK;
    }

    conn->reassembling = false;
    return SYN_INVALID_PARAM;
}

/** @brief Maximum L2CAP PDU bytes (including 4-byte header) fitting in HCI ACL TX buffer. */
#define L2CAP_MAX_TX_PDU                                                                        \
    (((SYN_BLE_HCI_TX_BUF_SIZE - 5U) < SYN_BLE_L2CAP_BUF_SIZE) ? (SYN_BLE_HCI_TX_BUF_SIZE - 5U) \
                                                               : SYN_BLE_L2CAP_BUF_SIZE)

SYN_Status syn_ble_l2cap_encode_pdu(uint16_t conn_handle, uint16_t cid, const uint8_t *payload,
                                    uint16_t len, uint8_t *tx_buf, uint16_t *tx_len)
{
    if (tx_buf == NULL || tx_len == NULL || (payload == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t l2cap_hdr[4];
    l2cap_hdr[0] = (uint8_t)(len & 0xFFU);
    l2cap_hdr[1] = (uint8_t)((len >> 8U) & 0xFFU);
    l2cap_hdr[2] = (uint8_t)(cid & 0xFFU);
    l2cap_hdr[3] = (uint8_t)((cid >> 8U) & 0xFFU);

    uint8_t l2cap_pdu[SYN_BLE_L2CAP_BUF_SIZE];
    if ((uint32_t)len + 4U > (uint32_t)L2CAP_MAX_TX_PDU) {
        return SYN_ERROR;
    }

    memcpy(&l2cap_pdu[0], l2cap_hdr, 4);
    if (len > 0U) {
        memcpy(&l2cap_pdu[4], payload, len);
    }

    return syn_ble_hci_encode_acl(conn_handle, 0x00U, 0x00U, l2cap_pdu, (uint16_t)(len + 4U),
                                  tx_buf, tx_len);
}
