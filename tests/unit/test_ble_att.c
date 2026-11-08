#include "syntropic/ble/syn_ble_att.h"
#include "unity/unity.h"

#include <string.h>

void test_ble_att_encoders(void)
{
    uint8_t buf[64];
    uint16_t len = 0;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_error_rsp(0, 0, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_mtu_rsp(0, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_read_rsp(NULL, 1, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_write_rsp(NULL, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_notification(1, NULL, 1, buf, &len));

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_att_encode_error_rsp(SYN_BLE_ATT_OP_READ_REQ, 0x0002,
                                                   SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(5, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_REQ, buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0x02, buf[2]);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, buf[4]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_mtu_rsp(64, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(3, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(64, buf[1]);

    uint8_t val[] = {0x01, 0x02};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_read_rsp(val, 2, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(3, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_RSP, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x01, buf[1]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_write_rsp(buf, &len));
    TEST_ASSERT_EQUAL_UINT16(1, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_WRITE_RSP, buf[0]);

    /* 0-length value payloads */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_read_rsp(NULL, 0, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(1, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_RSP, buf[0]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_notification(0x0004, NULL, 0, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(3, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_HANDLE_VAL_NOTIF, buf[0]);

    /* Oversized payload (> 250 bytes) */
    uint8_t large_val[255];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_att_encode_read_rsp(large_val, 251, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_notification(0x0005, large_val, 251, buf, &len));
}

void test_r5_ble_att_regressions(void)
{
    uint8_t buf[260];
    uint16_t len = 0U;
    uint8_t pair16[4] = {0x01U, 0x00U, 0x00U, 0x28U};
    uint8_t pair128[18] = {0};
    uint8_t big_buf[252] = {0};

    /* Find Information Response encoder */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_find_info_rsp(0x01U, NULL, 4U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_find_info_rsp(0x01U, pair16, 0U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_find_info_rsp(0x01U, big_buf, 252U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_find_info_rsp(0x03U, pair16, 4U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_find_info_rsp(0x01U, pair16, 3U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_find_info_rsp(SYN_BLE_ATT_FIND_INFO_FMT_UUID16,
                                                               pair16, 4U, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(6U, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_FIND_INFO_RSP, buf[0]);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_find_info_rsp(SYN_BLE_ATT_FIND_INFO_FMT_UUID128,
                                                               pair128, 18U, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(20U, len);

    /* Read By Type Response encoder */
    uint8_t type_item[7] = {0x02U, 0x00U, 0x02U, 0x03U, 0x00U, 0x6EU, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_type_rsp(7U, NULL, 7U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_type_rsp(2U, type_item, 2U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_type_rsp(7U, type_item, 5U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_type_rsp(4U, big_buf, 252U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_type_rsp(4U, type_item, 6U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_read_by_type_rsp(7U, type_item, 7U, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(9U, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_TYPE_RSP, buf[0]);

    /* Read By Group Type Response encoder */
    uint8_t grp_item[6] = {0x01U, 0x00U, 0x04U, 0x00U, 0x09U, 0x18U};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_group_rsp(6U, NULL, 6U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_group_rsp(4U, grp_item, 4U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_group_rsp(6U, grp_item, 3U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_group_rsp(6U, big_buf, 252U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_att_encode_read_by_group_rsp(5U, grp_item, 6U, buf, &len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_att_encode_read_by_group_rsp(6U, grp_item, 6U, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(8U, len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP, buf[0]);
}
