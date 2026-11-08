/**
 * @file syn_ble_att.c
 * @brief Zero-Heap Attribute Protocol (ATT) Implementation.
 */

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

SYN_Status syn_ble_att_encode_find_info_rsp(uint8_t format, const uint8_t *info_data,
                                            uint16_t info_len, uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL || info_data == NULL || info_len == 0U || info_len > 248U) {
        return SYN_INVALID_PARAM;
    }

    uint16_t stride = 0U;
    if (format == SYN_BLE_ATT_FIND_INFO_FMT_UUID16) {
        stride = 4U;
    } else if (format == SYN_BLE_ATT_FIND_INFO_FMT_UUID128) {
        stride = 18U;
    } else {
        return SYN_INVALID_PARAM;
    }

    if ((info_len % stride) != 0U) {
        return SYN_INVALID_PARAM;
    }

    buf[0] = SYN_BLE_ATT_OP_FIND_INFO_RSP;
    buf[1] = format;
    memcpy(&buf[2], info_data, info_len);
    *len = (uint16_t)(2U + info_len);
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_read_by_type_rsp(uint8_t pair_len, const uint8_t *data_list,
                                               uint16_t list_len, uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL || data_list == NULL || pair_len < 3U ||
        list_len < (uint16_t)pair_len || list_len > 248U || (list_len % (uint16_t)pair_len) != 0U) {
        return SYN_INVALID_PARAM;
    }

    buf[0] = SYN_BLE_ATT_OP_READ_BY_TYPE_RSP;
    buf[1] = pair_len;
    memcpy(&buf[2], data_list, list_len);
    *len = (uint16_t)(2U + list_len);
    return SYN_OK;
}

SYN_Status syn_ble_att_encode_read_by_group_rsp(uint8_t data_len, const uint8_t *data_list,
                                                uint16_t list_len, uint8_t *buf, uint16_t *len)
{
    if (buf == NULL || len == NULL || data_list == NULL || data_len < 5U ||
        list_len < (uint16_t)data_len || list_len > 248U || (list_len % (uint16_t)data_len) != 0U) {
        return SYN_INVALID_PARAM;
    }

    buf[0] = SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP;
    buf[1] = data_len;
    memcpy(&buf[2], data_list, list_len);
    *len = (uint16_t)(2U + list_len);
    return SYN_OK;
}
