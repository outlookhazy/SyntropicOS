

# File syn\_ble\_gatt.c

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gatt.c**](syn__ble__gatt_8c.md)

[Go to the documentation of this file](syn__ble__gatt_8c.md)


```C++

#include "syn_ble_gatt.h"

#include <string.h>

SYN_Status syn_ble_gatt_init(SYN_BLE_GATT *gatt, const SYN_BLE_GATT_Attr *table, uint16_t table_len,
                             void *user_data)
{
    if (gatt == NULL || (table == NULL && table_len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    memset(gatt, 0, sizeof(*gatt));
    gatt->table = table;
    gatt->table_len = table_len;
    gatt->user_data = user_data;
    return SYN_OK;
}

static const SYN_BLE_GATT_Attr *find_attr(const SYN_BLE_GATT *gatt, uint16_t handle)
{
    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        if (gatt->table[i].handle == handle) {
            return &gatt->table[i];
        }
    }
    return NULL;
}

SYN_Status syn_ble_gatt_process_att_pdu(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                        uint16_t *resp_len)
{
    if (gatt == NULL || att_pdu == NULL || resp_buf == NULL || resp_len == NULL || pdu_len < 1U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t opcode = att_pdu[0];

    if (opcode == SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ && pdu_len >= 3U) {
        return syn_ble_att_encode_mtu_rsp(SYN_BLE_ATT_MTU_DEFAULT, resp_buf, resp_len);
    }

    if (opcode == SYN_BLE_ATT_OP_READ_REQ && pdu_len >= 3U) {
        uint16_t handle = (uint16_t)(att_pdu[1] | (att_pdu[2] << 8U));
        const SYN_BLE_GATT_Attr *attr = find_attr(gatt, handle);

        if (attr == NULL) {
            return syn_ble_att_encode_error_rsp(opcode, handle, SYN_BLE_ATT_ERR_ATTR_NOT_FOUND,
                                                resp_buf, resp_len);
        }

        uint8_t val[64];
        uint16_t val_len = 0;
        SYN_Status st = SYN_OK;

        if (attr->read_cb != NULL) {
            st = attr->read_cb(gatt, conn_handle, handle, val, &val_len, gatt->user_data);
        } else if (attr->static_val != NULL) {
            val_len = attr->static_val_len;
            if (val_len > sizeof(val)) {
                val_len = sizeof(val);
            }
            memcpy(val, attr->static_val, val_len);
        } else {
            return syn_ble_att_encode_error_rsp(opcode, handle, SYN_BLE_ATT_ERR_READ_NOT_PERMITTED,
                                                resp_buf, resp_len);
        }

        if (st == SYN_OK) {
            return syn_ble_att_encode_read_rsp(val, val_len, resp_buf, resp_len);
        } else {
            return syn_ble_att_encode_error_rsp(opcode, handle, SYN_BLE_ATT_ERR_UNLIKELY_ERROR,
                                                resp_buf, resp_len);
        }
    }

    if ((opcode == SYN_BLE_ATT_OP_WRITE_REQ || opcode == SYN_BLE_ATT_OP_WRITE_CMD) &&
        pdu_len >= 3U) {
        uint16_t handle = (uint16_t)(att_pdu[1] | (att_pdu[2] << 8U));
        const SYN_BLE_GATT_Attr *attr = find_attr(gatt, handle);

        if (attr == NULL) {
            if (opcode == SYN_BLE_ATT_OP_WRITE_REQ) {
                return syn_ble_att_encode_error_rsp(opcode, handle, SYN_BLE_ATT_ERR_ATTR_NOT_FOUND,
                                                    resp_buf, resp_len);
            }
            *resp_len = 0;
            return SYN_OK;
        }

        const uint8_t *val = &att_pdu[3];
        uint16_t val_len = (uint16_t)(pdu_len - 3U);
        SYN_Status st = SYN_OK;

        if (attr->write_cb != NULL) {
            st = attr->write_cb(gatt, conn_handle, handle, val, val_len, gatt->user_data);
        }

        if (opcode == SYN_BLE_ATT_OP_WRITE_REQ) {
            if (st == SYN_OK) {
                return syn_ble_att_encode_write_rsp(resp_buf, resp_len);
            } else {
                return syn_ble_att_encode_error_rsp(
                    opcode, handle, SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED, resp_buf, resp_len);
            }
        } else {
            *resp_len = 0;
            return SYN_OK;
        }
    }

    return syn_ble_att_encode_error_rsp(opcode, 0x0000U, SYN_BLE_ATT_ERR_REQ_NOT_SUPPORTED,
                                        resp_buf, resp_len);
}

SYN_Status syn_ble_gatt_notify(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                               const uint8_t *val, uint16_t val_len, uint8_t *tx_buf,
                               uint16_t *tx_len)
{
    if (gatt == NULL || tx_buf == NULL || tx_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t att_pdu[SYN_BLE_L2CAP_BUF_SIZE];
    uint16_t att_len = 0;
    SYN_Status st = syn_ble_att_encode_notification(attr_handle, val, val_len, att_pdu, &att_len);
    if (st != SYN_OK) {
        return st;
    }

    return syn_ble_l2cap_encode_pdu(conn_handle, SYN_BLE_L2CAP_CID_ATT, att_pdu, att_len, tx_buf,
                                    tx_len);
}
```


