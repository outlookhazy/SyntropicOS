/**
 * @file syn_ble_gatt.c
 * @brief Zero-Heap Generic Attribute Profile (GATT) Implementation.
 */

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

/**
 * @brief Decode little-endian 16-bit integer from buffer.
 * @param p 2-byte pointer.
 * @return Decoded uint16_t.
 */
static uint16_t gatt_get_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8U));
}

/**
 * @brief Encode little-endian 16-bit integer into buffer.
 * @param p 2-byte output pointer.
 * @param v Value to encode.
 */
static void gatt_put_le16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8U) & 0xFFU);
}

/**
 * @brief Internal attribute lookup helper.
 * @param gatt GATT server handle.
 * @param handle Attribute handle.
 * @return Attribute pointer or NULL.
 */
static const SYN_BLE_GATT_Attr *find_attr(const SYN_BLE_GATT *gatt, uint16_t handle)
{
    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        if (gatt->table[i].handle == handle) {
            return &gatt->table[i];
        }
    }
    return NULL;
}

/**
 * @brief Resolve per-CCCD slot index for a 0x2902 descriptor handle.
 * @param gatt GATT server handle.
 * @param handle CCCD attribute handle.
 * @return CCCD slot index in [0, SYN_BLE_MAX_CCCDS - 1].
 */
static uint8_t gatt_cccd_slot(const SYN_BLE_GATT *gatt, uint16_t handle)
{
    uint8_t slot = 0U;
    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        if (gatt->table[i].uuid16 == SYN_BLE_UUID_CCCD) {
            if (gatt->table[i].handle == handle) {
                break;
            }
            if ((uint8_t)(slot + 1U) < SYN_BLE_MAX_CCCDS) {
                slot++;
            }
        }
    }
    return slot;
}

/**
 * @brief Read raw attribute value via callback, static buffer, or CCCD state.
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param attr Attribute descriptor.
 * @param val Output value buffer (must be at least SYN_BLE_GATT_MAX_ATTR_LEN bytes).
 * @param max_len Capacity of val buffer.
 * @param val_len Output value byte length.
 * @return SYN_OK on success, SYN_INVALID_PARAM if no value source, or callback error.
 */
static SYN_Status gatt_read_attr_val(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                     const SYN_BLE_GATT_Attr *attr, uint8_t *val, uint16_t max_len,
                                     uint16_t *val_len)
{
    if (attr->read_cb != NULL) {
        SYN_Status st =
            attr->read_cb(gatt, conn_handle, attr->handle, val, val_len, gatt->user_data);
        if (st == SYN_OK && *val_len > max_len) {
            *val_len = max_len;
        }
        return st;
    }
    if (attr->static_val != NULL) {
        uint16_t copy_len = attr->static_val_len;
        if (copy_len > max_len) {
            copy_len = max_len;
        }
        memcpy(val, attr->static_val, copy_len);
        *val_len = copy_len;
        return SYN_OK;
    }
    if (attr->uuid16 == SYN_BLE_UUID_CCCD) {
        uint8_t slot = gatt_cccd_slot(gatt, attr->handle);
        uint16_t cccd = gatt->cccd_vals[conn_handle % SYN_BLE_MAX_CONNECTIONS][slot];
        gatt_put_le16(val, cccd);
        *val_len = 2U;
        return SYN_OK;
    }
    return SYN_INVALID_PARAM;
}

/**
 * @brief Handle ATT Read Request (0x0A).
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_handle_read_req(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                       const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                       uint16_t *resp_len)
{
    if (pdu_len != 3U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, 0x0000U,
                                            SYN_BLE_ATT_ERR_INVALID_PDU, resp_buf, resp_len);
    }

    uint16_t handle = gatt_get_le16(&att_pdu[1]);
    if (handle == 0x0000U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, 0x0000U,
                                            SYN_BLE_ATT_ERR_INVALID_HANDLE, resp_buf, resp_len);
    }

    const SYN_BLE_GATT_Attr *attr = find_attr(gatt, handle);
    if (attr == NULL) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, handle,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }
    if ((attr->permissions & SYN_BLE_PROP_READ) == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, handle,
                                            SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp_buf, resp_len);
    }

    uint8_t val[SYN_BLE_GATT_MAX_ATTR_LEN];
    uint16_t val_len = 0U;
    SYN_Status st =
        gatt_read_attr_val(gatt, conn_handle, attr, val, (uint16_t)sizeof(val), &val_len);
    if (st == SYN_INVALID_PARAM) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, handle,
                                            SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp_buf, resp_len);
    }
    if (st != SYN_OK) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, handle,
                                            SYN_BLE_ATT_ERR_UNLIKELY_ERROR, resp_buf, resp_len);
    }
    if (val_len > (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 1U)) {
        val_len = (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 1U);
    }

    return syn_ble_att_encode_read_rsp(val, val_len, resp_buf, resp_len);
}

/**
 * @brief Handle ATT Write Request (0x12).
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_handle_write_req(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                        uint16_t *resp_len)
{
    if (pdu_len < 3U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, 0x0000U,
                                            SYN_BLE_ATT_ERR_INVALID_PDU, resp_buf, resp_len);
    }

    uint16_t handle = gatt_get_le16(&att_pdu[1]);
    if (handle == 0x0000U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, 0x0000U,
                                            SYN_BLE_ATT_ERR_INVALID_HANDLE, resp_buf, resp_len);
    }

    const SYN_BLE_GATT_Attr *attr = find_attr(gatt, handle);
    if (attr == NULL) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, handle,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }
    if ((attr->permissions & SYN_BLE_PROP_WRITE) == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, handle,
                                            SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED, resp_buf,
                                            resp_len);
    }

    const uint8_t *val = &att_pdu[3];
    uint16_t val_len = (uint16_t)(pdu_len - 3U);

    if (attr->write_cb != NULL) {
        if (attr->write_cb(gatt, conn_handle, handle, val, val_len, gatt->user_data) != SYN_OK) {
            return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, handle,
                                                SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED, resp_buf,
                                                resp_len);
        }
    } else if (attr->uuid16 == SYN_BLE_UUID_CCCD) {
        if (val_len != 2U) {
            return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_WRITE_REQ, handle,
                                                SYN_BLE_ATT_ERR_INVALID_ATTR_LEN, resp_buf,
                                                resp_len);
        }
        uint8_t slot = gatt_cccd_slot(gatt, handle);
        gatt->cccd_vals[conn_handle % SYN_BLE_MAX_CONNECTIONS][slot] = gatt_get_le16(val);
    }

    return syn_ble_att_encode_write_rsp(resp_buf, resp_len);
}

/**
 * @brief Handle ATT Write Command (0x52) without response.
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param att_pdu Incoming ATT command PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_len Output response byte length (set to 0).
 * @return SYN_OK.
 */
static SYN_Status gatt_handle_write_cmd(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len,
                                        uint16_t *resp_len)
{
    *resp_len = 0U;
    if (pdu_len < 3U) {
        return SYN_OK;
    }

    uint16_t handle = gatt_get_le16(&att_pdu[1]);
    if (handle == 0x0000U) {
        return SYN_OK;
    }

    const SYN_BLE_GATT_Attr *attr = find_attr(gatt, handle);
    if (attr == NULL || (attr->permissions & SYN_BLE_PROP_WRITE_CMD) == 0U) {
        return SYN_OK;
    }

    const uint8_t *val = &att_pdu[3];
    uint16_t val_len = (uint16_t)(pdu_len - 3U);
    if (attr->write_cb != NULL) {
        (void)attr->write_cb(gatt, conn_handle, handle, val, val_len, gatt->user_data);
    } else if (attr->uuid16 == SYN_BLE_UUID_CCCD && val_len == 2U) {
        uint8_t slot = gatt_cccd_slot(gatt, handle);
        gatt->cccd_vals[conn_handle % SYN_BLE_MAX_CONNECTIONS][slot] = gatt_get_le16(val);
    }
    return SYN_OK;
}

/**
 * @brief Validate common handle range header for ATT discovery requests.
 * @param opcode ATT request opcode for error responses.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Actual PDU byte length.
 * @param expected_len Expected 16-bit UUID PDU byte length.
 * @param allow_uuid128 Allow 21-byte 128-bit UUID discovery requests.
 * @param start_h Output decoded start handle.
 * @param end_h Output decoded end handle.
 * @param resp_buf Output ATT error response buffer on validation failure.
 * @param resp_len Output ATT error response byte length on validation failure.
 * @return true if PDU length and handle range are valid, false otherwise.
 */
static bool gatt_validate_range(uint8_t opcode, const uint8_t *att_pdu, uint16_t pdu_len,
                                uint16_t expected_len, bool allow_uuid128, uint16_t *start_h,
                                uint16_t *end_h, uint8_t *resp_buf, uint16_t *resp_len)
{
    bool len_ok = (pdu_len == expected_len) || (allow_uuid128 && (pdu_len == 21U));
    if (!len_ok) {
        (void)syn_ble_att_encode_error_rsp(opcode, 0x0000U, SYN_BLE_ATT_ERR_INVALID_PDU, resp_buf,
                                           resp_len);
        return false;
    }

    *start_h = gatt_get_le16(&att_pdu[1]);
    *end_h = gatt_get_le16(&att_pdu[3]);
    if (*start_h == 0x0000U || *start_h > *end_h) {
        (void)syn_ble_att_encode_error_rsp(opcode, *start_h, SYN_BLE_ATT_ERR_INVALID_HANDLE,
                                           resp_buf, resp_len);
        return false;
    }
    return true;
}

/**
 * @brief Check whether attribute handle falls within non-zero [start_h, end_h].
 * @param attr Attribute descriptor.
 * @param start_h Inclusive start handle.
 * @param end_h Inclusive end handle.
 * @return true if attribute handle is non-zero and within range, false otherwise.
 */
static bool gatt_attr_in_range(const SYN_BLE_GATT_Attr *attr, uint16_t start_h, uint16_t end_h)
{
    return (attr->handle != 0x0000U) && (attr->handle >= start_h) && (attr->handle <= end_h);
}

/**
 * @brief Handle ATT Find Information Request (0x04) for handles and CCCD (0x2902).
 * @param gatt GATT server handle.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_handle_find_info(const SYN_BLE_GATT *gatt, const uint8_t *att_pdu,
                                        uint16_t pdu_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint16_t start_h = 0U;
    uint16_t end_h = 0U;
    if (!gatt_validate_range(SYN_BLE_ATT_OP_FIND_INFO_REQ, att_pdu, pdu_len, 5U, false, &start_h,
                             &end_h, resp_buf, resp_len)) {
        return SYN_OK;
    }

    uint8_t info_buf[SYN_BLE_ATT_MTU_DEFAULT];
    uint16_t info_len = 0U;
    const uint16_t max_info_len = (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 2U);

    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        const SYN_BLE_GATT_Attr *attr = &gatt->table[i];
        if (gatt_attr_in_range(attr, start_h, end_h)) {
            if ((uint16_t)(info_len + 4U) > max_info_len) {
                break;
            }
            gatt_put_le16(&info_buf[info_len], attr->handle);
            gatt_put_le16(&info_buf[info_len + 2U], attr->uuid16);
            info_len = (uint16_t)(info_len + 4U);
        }
    }

    if (info_len == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_FIND_INFO_REQ, start_h,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }

    return syn_ble_att_encode_find_info_rsp(SYN_BLE_ATT_FIND_INFO_FMT_UUID16, info_buf, info_len,
                                            resp_buf, resp_len);
}

/**
 * @brief Compute end group handle for a service attribute at table index svc_idx.
 * @param gatt GATT server handle.
 * @param svc_idx Service attribute index in gatt->table.
 * @return Last attribute handle belonging to the service group.
 */
static uint16_t gatt_service_end_handle(const SYN_BLE_GATT *gatt, uint16_t svc_idx)
{
    uint16_t end_h = gatt->table[svc_idx].handle;
    for (uint16_t j = (uint16_t)(svc_idx + 1U); j < gatt->table_len; j++) {
        uint16_t u = gatt->table[j].uuid16;
        if (u == SYN_BLE_UUID_PRIMARY_SERVICE || u == SYN_BLE_UUID_SECONDARY_SERVICE) {
            break;
        }
        if (gatt->table[j].handle > end_h) {
            end_h = gatt->table[j].handle;
        }
    }
    return end_h;
}

/**
 * @brief Handle ATT Read By Group Type Request (0x10) for Primary/Secondary Services.
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_handle_read_by_group(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                            const uint8_t *att_pdu, uint16_t pdu_len,
                                            uint8_t *resp_buf, uint16_t *resp_len)
{
    uint16_t start_h = 0U;
    uint16_t end_h = 0U;
    if (!gatt_validate_range(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, att_pdu, pdu_len, 7U, true,
                             &start_h, &end_h, resp_buf, resp_len)) {
        return SYN_OK;
    }
    if (pdu_len == 21U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE, resp_buf,
                                            resp_len);
    }

    uint16_t group_uuid = gatt_get_le16(&att_pdu[5]);
    if (group_uuid != SYN_BLE_UUID_PRIMARY_SERVICE &&
        group_uuid != SYN_BLE_UUID_SECONDARY_SERVICE) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE, resp_buf,
                                            resp_len);
    }

    uint8_t list_buf[SYN_BLE_ATT_MTU_DEFAULT];
    uint16_t list_len = 0U;
    uint8_t elem_len = 0U;
    const uint16_t max_list_len = (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 2U);

    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        const SYN_BLE_GATT_Attr *attr = &gatt->table[i];
        if (!gatt_attr_in_range(attr, start_h, end_h) || attr->uuid16 != group_uuid) {
            continue;
        }

        uint8_t svc_val[SYN_BLE_GATT_MAX_ATTR_LEN];
        uint16_t svc_val_len = 0U;
        if (gatt_read_attr_val(gatt, conn_handle, attr, svc_val, (uint16_t)sizeof(svc_val),
                               &svc_val_len) != SYN_OK ||
            (svc_val_len != 2U && svc_val_len != 16U)) {
            continue;
        }

        uint8_t cur_elem_len = (uint8_t)(4U + svc_val_len);
        if (elem_len == 0U) {
            elem_len = cur_elem_len;
        } else if (cur_elem_len != elem_len) {
            break;
        }

        if ((uint16_t)(list_len + (uint16_t)cur_elem_len) > max_list_len) {
            break;
        }

        uint16_t grp_end = gatt_service_end_handle(gatt, i);
        gatt_put_le16(&list_buf[list_len], attr->handle);
        gatt_put_le16(&list_buf[list_len + 2U], grp_end);
        memcpy(&list_buf[list_len + 4U], svc_val, svc_val_len);
        list_len = (uint16_t)(list_len + (uint16_t)cur_elem_len);
    }

    if (list_len == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }

    return syn_ble_att_encode_read_by_group_rsp(elem_len, list_buf, list_len, resp_buf, resp_len);
}

/**
 * @brief Build 5-byte Characteristic Declaration payload for 0x2803 row idx.
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param idx Attribute table row index.
 * @param decl_val Output 5-byte Characteristic Declaration value buffer.
 * @return true if row idx is a valid 0x2803 declaration with decl_handle < val_handle.
 */
static bool gatt_extract_char_decl(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t idx,
                                   uint8_t decl_val[5])
{
    const SYN_BLE_GATT_Attr *attr = &gatt->table[idx];
    if (attr->uuid16 != SYN_BLE_UUID_CHARACTERISTIC) {
        return false;
    }

    uint8_t tmp_val[SYN_BLE_GATT_MAX_ATTR_LEN];
    uint16_t vlen = 0U;
    if (gatt_read_attr_val(gatt, conn_handle, attr, tmp_val, (uint16_t)sizeof(tmp_val), &vlen) ==
            SYN_OK &&
        vlen == 5U) {
        if (gatt_get_le16(&tmp_val[1]) <= attr->handle) {
            return false;
        }
        memcpy(decl_val, tmp_val, 5U);
        return true;
    }

    if ((uint16_t)(idx + 1U) < gatt->table_len) {
        const SYN_BLE_GATT_Attr *val_attr = &gatt->table[idx + 1U];
        if (val_attr->handle <= attr->handle) {
            return false;
        }
        decl_val[0] = val_attr->permissions;
        gatt_put_le16(&decl_val[1], val_attr->handle);
        gatt_put_le16(&decl_val[3], val_attr->uuid16);
        return true;
    }
    return false;
}

/**
 * @brief Handle Characteristic Declaration (0x2803) Read By Type discovery.
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param start_h Inclusive start handle.
 * @param end_h Inclusive end handle.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_read_char_decls(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t start_h,
                                       uint16_t end_h, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t list_buf[SYN_BLE_ATT_MTU_DEFAULT];
    uint16_t list_len = 0U;
    const uint16_t max_list_len = (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 2U);

    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        const SYN_BLE_GATT_Attr *attr = &gatt->table[i];
        uint8_t decl_val[5];
        if (!gatt_attr_in_range(attr, start_h, end_h) ||
            !gatt_extract_char_decl(gatt, conn_handle, i, decl_val)) {
            continue;
        }
        if ((uint16_t)(list_len + 7U) > max_list_len) {
            break;
        }
        gatt_put_le16(&list_buf[list_len], attr->handle);
        memcpy(&list_buf[list_len + 2U], decl_val, 5U);
        list_len = (uint16_t)(list_len + 7U);
    }

    if (list_len == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }
    return syn_ble_att_encode_read_by_type_rsp(7U, list_buf, list_len, resp_buf, resp_len);
}

/**
 * @brief Handle ATT Read By Type Request (0x08).
 * @param gatt GATT server handle.
 * @param conn_handle Connection handle.
 * @param att_pdu Incoming ATT request PDU.
 * @param pdu_len Length of att_pdu in bytes.
 * @param resp_buf Output ATT response buffer.
 * @param resp_len Output ATT response byte length.
 * @return SYN_OK on encoded response or error code on invalid parameters.
 */
static SYN_Status gatt_handle_read_by_type(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                           const uint8_t *att_pdu, uint16_t pdu_len,
                                           uint8_t *resp_buf, uint16_t *resp_len)
{
    uint16_t start_h = 0U;
    uint16_t end_h = 0U;
    if (!gatt_validate_range(SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, att_pdu, pdu_len, 7U, true, &start_h,
                             &end_h, resp_buf, resp_len)) {
        return SYN_OK;
    }
    if (pdu_len == 21U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }

    uint16_t type_uuid = gatt_get_le16(&att_pdu[5]);
    if (type_uuid == SYN_BLE_UUID_CHARACTERISTIC) {
        return gatt_read_char_decls(gatt, conn_handle, start_h, end_h, resp_buf, resp_len);
    }

    uint8_t list_buf[SYN_BLE_ATT_MTU_DEFAULT];
    uint16_t list_len = 0U;
    uint8_t pair_len = 0U;
    const uint16_t max_list_len = (uint16_t)(SYN_BLE_ATT_MTU_DEFAULT - 2U);

    for (uint16_t i = 0U; i < gatt->table_len; i++) {
        const SYN_BLE_GATT_Attr *attr = &gatt->table[i];
        if (!gatt_attr_in_range(attr, start_h, end_h) || attr->uuid16 != type_uuid) {
            continue;
        }
        if ((attr->permissions & SYN_BLE_PROP_READ) == 0U) {
            if (list_len == 0U) {
                return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, attr->handle,
                                                    SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp_buf,
                                                    resp_len);
            }
            break;
        }

        uint8_t val[SYN_BLE_GATT_MAX_ATTR_LEN];
        uint16_t val_len = 0U;
        if (gatt_read_attr_val(gatt, conn_handle, attr, val, (uint16_t)sizeof(val), &val_len) !=
                SYN_OK ||
            val_len == 0U) {
            continue;
        }
        if (val_len > (uint16_t)(max_list_len - 2U)) {
            val_len = (uint16_t)(max_list_len - 2U);
        }

        uint8_t cur_pair_len = (uint8_t)(2U + val_len);
        if (pair_len == 0U) {
            pair_len = cur_pair_len;
        } else if (cur_pair_len != pair_len) {
            break;
        }
        if ((uint16_t)(list_len + (uint16_t)cur_pair_len) > max_list_len) {
            break;
        }

        gatt_put_le16(&list_buf[list_len], attr->handle);
        memcpy(&list_buf[list_len + 2U], val, val_len);
        list_len = (uint16_t)(list_len + (uint16_t)cur_pair_len);
    }

    if (list_len == 0U) {
        return syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, start_h,
                                            SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp_buf, resp_len);
    }
    return syn_ble_att_encode_read_by_type_rsp(pair_len, list_buf, list_len, resp_buf, resp_len);
}

SYN_Status syn_ble_gatt_process_att_pdu(SYN_BLE_GATT *gatt, uint16_t conn_handle,
                                        const uint8_t *att_pdu, uint16_t pdu_len, uint8_t *resp_buf,
                                        uint16_t *resp_len)
{
    if (gatt == NULL || att_pdu == NULL || resp_buf == NULL || resp_len == NULL || pdu_len < 1U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t opcode = att_pdu[0];
    switch (opcode) {
    case SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ:
        if (pdu_len != 3U) {
            return syn_ble_att_encode_error_rsp(opcode, 0x0000U, SYN_BLE_ATT_ERR_INVALID_PDU,
                                                resp_buf, resp_len);
        }
        return syn_ble_att_encode_mtu_rsp(SYN_BLE_ATT_MTU_DEFAULT, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_FIND_INFO_REQ:
        return gatt_handle_find_info(gatt, att_pdu, pdu_len, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_READ_BY_TYPE_REQ:
        return gatt_handle_read_by_type(gatt, conn_handle, att_pdu, pdu_len, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_READ_REQ:
        return gatt_handle_read_req(gatt, conn_handle, att_pdu, pdu_len, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ:
        return gatt_handle_read_by_group(gatt, conn_handle, att_pdu, pdu_len, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_WRITE_REQ:
        return gatt_handle_write_req(gatt, conn_handle, att_pdu, pdu_len, resp_buf, resp_len);

    case SYN_BLE_ATT_OP_WRITE_CMD:
        return gatt_handle_write_cmd(gatt, conn_handle, att_pdu, pdu_len, resp_len);

    default:
        if (((opcode & 0x01U) != 0U) || ((opcode & 0x40U) != 0U) || (opcode == 0x1EU)) {
            *resp_len = 0U;
            return SYN_OK;
        }
        return syn_ble_att_encode_error_rsp(opcode, 0x0000U, SYN_BLE_ATT_ERR_REQ_NOT_SUPPORTED,
                                            resp_buf, resp_len);
    }
}

SYN_Status syn_ble_gatt_notify(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                               const uint8_t *val, uint16_t val_len, uint8_t *tx_buf,
                               uint16_t *tx_len)
{
    if (gatt == NULL || tx_buf == NULL || tx_len == NULL ||
        val_len > (SYN_BLE_ATT_MTU_DEFAULT - 3U)) {
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
