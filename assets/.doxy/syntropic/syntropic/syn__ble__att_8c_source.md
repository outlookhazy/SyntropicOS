

# File syn\_ble\_att.c

[**File List**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_att.c**](syn__ble__att_8c.md)

[Go to the documentation of this file](syn__ble__att_8c.md)


```C++

#include "syn_ble_att.h"

#include <string.h>

SYN_Status syn_ble_att_encode_error_rsp(uint8_t req_opcode, uint16_t handle, uint8_t err_code,
                                        uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL) {
        return SYN_INVALID_PARAM;
    }
    buf[0] = SYN_BLE_ATT_OP_ERROR_RSP;
    buf[1] = req_opcode;
    buf[2] = (uint8_t)(handle & 0xFFU);
    buf[3] = (uint8_t)((handle >> 8U) & 0xFFU);
    buf[4] = err_code;
    *len = 5U;
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_mtu_rsp(uint16_t server_mtu, uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL) {
        return SYN_INVALID_PARAM;
    }
    buf[0] = SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP;
    buf[1] = (uint8_t)(server_mtu & 0xFFU);
    buf[2] = (uint8_t)((server_mtu >> 8U) & 0xFFU);
    *len = 3U;
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_read_rsp(const uint8_t *val, uint16_t val_len, uint8_t *buf,
                                       uint16_t *len)
{
    if (buf == NULL || len == NULL || (val == NULL && val_len > 0U) || val_len > 250U) {
        return SYN_INVALID_PARAM;
    }
    buf[0] = SYN_BLE_ATT_OP_READ_RSP;
    if (val_len > 0U) {
        memcpy(&buf[1], val, val_len);
    }
    *len = (uint16_t)(1U + val_len);
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_write_rsp(uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL) {
        return SYN_INVALID_PARAM;
    }
    buf[0] = SYN_BLE_ATT_OP_WRITE_RSP;
    *len = 1U;
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_notification(uint16_t handle, const uint8_t *val, uint16_t val_len,
                                           uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL || (val == NULL && val_len > 0U) || val_len > 250U) {
        return SYN_INVALID_PARAM;
    }
    buf[0] = SYN_BLE_ATT_OP_HANDLE_VAL_NOTIF;
    buf[1] = (uint8_t)(handle & 0xFFU);
    buf[2] = (uint8_t)((handle >> 8U) & 0xFFU);
    if (val_len > 0U) {
        memcpy(&buf[3], val, val_len);
    }
    *len = (uint16_t)(3U + val_len);
    return SYN_OK;
}
```


