#include "syntropic/ble/syn_ble_gatt.h"
#include "unity/unity.h"

#include <string.h>

static uint8_t mock_temp = 25;
static bool mock_read_fail = false;
static bool mock_write_fail = false;
static uint8_t large_static_buf[100];

static SYN_Status long_dyn_read_cb(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                                   uint8_t *val, uint16_t *val_len, void *user_data)
{
    (void)gatt;
    (void)conn_handle;
    (void)attr_handle;
    (void)user_data;
    memset(val, 0x5AU, 48U);
    *val_len = 80U; /* Exceeds 64B scratch + 23B MTU to verify clamp safety */
    return SYN_OK;
}

static SYN_Status temp_read_cb(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                               uint8_t *val, uint16_t *val_len, void *user_data)
{
    (void)gatt;
    (void)conn_handle;
    (void)attr_handle;
    (void)user_data;
    if (mock_read_fail) {
        return SYN_ERROR;
    }
    val[0] = mock_temp;
    *val_len = 1;
    return SYN_OK;
}

static SYN_Status temp_write_cb(SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle,
                                const uint8_t *val, uint16_t val_len, void *user_data)
{
    (void)gatt;
    (void)conn_handle;
    (void)attr_handle;
    (void)user_data;
    if (mock_write_fail) {
        return SYN_ERROR;
    }
    if (val_len > 0) {
        mock_temp = val[0];
    }
    return SYN_OK;
}

static const SYN_BLE_GATT_Attr test_gatt_table[] = {
    {0x0001, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL,
     (const uint8_t *)"\x09\x18", 2},
    {0x0002, 0x2A6E, SYN_BLE_PROP_READ | SYN_BLE_PROP_WRITE | SYN_BLE_PROP_WRITE_CMD, temp_read_cb,
     temp_write_cb, NULL, 0},
    {0x0003, 0x2A00, 0, NULL, NULL, NULL, 0}, /* No read, no write, no static val */
    {0x0004, 0x2A01, SYN_BLE_PROP_READ, NULL, NULL, large_static_buf,
     100} /* Oversized static val (>64B) */
};

void test_ble_gatt_null_params(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gatt_init(NULL, NULL, 0, NULL));

    SYN_BLE_GATT gatt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&gatt, test_gatt_table, 4, NULL));

    uint8_t resp[128];
    uint16_t resp_len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_gatt_process_att_pdu(NULL, 1, NULL, 0, resp, &resp_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_gatt_notify(NULL, 1, 0x0002, NULL, 0, NULL, &resp_len));
}

void test_ble_gatt_read_request(void)
{
    SYN_BLE_GATT gatt;
    syn_ble_gatt_init(&gatt, test_gatt_table, 4, NULL);

    /* Static val read */
    uint8_t static_read_req[] = {SYN_BLE_ATT_OP_READ_REQ, 0x01, 0x00};
    uint8_t resp[128];
    uint16_t resp_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1, static_read_req,
                                                   sizeof(static_read_req), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_RSP, resp[0]);

    /* Oversized static val read (clamped to ATT_MTU - 1 = 22B per Core Spec Vol 3 Part F §3.4.4.4)
     */
    uint8_t large_read_req[] = {SYN_BLE_ATT_OP_READ_REQ, 0x04, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1, large_read_req, sizeof(large_read_req),
                                                   resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(SYN_BLE_ATT_MTU_DEFAULT, resp_len); /* 1B opcode + 22B val */

    /* Callback read success */
    mock_read_fail = false;
    uint8_t read_req[] = {SYN_BLE_ATT_OP_READ_REQ, 0x02, 0x00};
    mock_temp = 25;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, read_req, sizeof(read_req),
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(2, resp_len);
    TEST_ASSERT_EQUAL_UINT8(25, resp[1]);

    /* Callback read failure */
    mock_read_fail = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, read_req, sizeof(read_req),
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
    mock_read_fail = false;

    /* Not found handle read */
    uint8_t not_found_req[] = {SYN_BLE_ATT_OP_READ_REQ, 0x99, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, not_found_req,
                                                           sizeof(not_found_req), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);

    /* Read not permitted attribute */
    uint8_t no_perm_req[] = {SYN_BLE_ATT_OP_READ_REQ, 0x03, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, no_perm_req,
                                                           sizeof(no_perm_req), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
}

void test_ble_gatt_write_request(void)
{
    SYN_BLE_GATT gatt;
    syn_ble_gatt_init(&gatt, test_gatt_table, 4, NULL);

    /* Write success */
    mock_write_fail = false;
    uint8_t write_req[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x02, 0x00, 30};
    uint8_t resp[128];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, write_req, sizeof(write_req),
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(1, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_WRITE_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(30, mock_temp);

    /* Write request on non-existent handle */
    uint8_t write_unmapped_req[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x99, 0x00, 30};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1, write_unmapped_req,
                                                   sizeof(write_unmapped_req), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    /* Write failure */
    mock_write_fail = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, write_req, sizeof(write_req),
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
    mock_write_fail = false;

    /* Write command on unknown handle */
    uint8_t write_cmd_unknown[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x99, 0x00, 35};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1, write_cmd_unknown,
                                                   sizeof(write_cmd_unknown), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    /* Write command (no response) */
    uint8_t write_cmd[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x02, 0x00, 35};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, write_cmd, sizeof(write_cmd),
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);
    TEST_ASSERT_EQUAL_UINT8(35, mock_temp);

    /* Exchange MTU request */
    uint8_t mtu_req[] = {SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ, 0x40, 0x00};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, mtu_req, sizeof(mtu_req), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP, resp[0]);

    /* Unsupported PDU opcode */
    uint8_t unknown_pdu[] = {0xFE, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1, unknown_pdu,
                                                           sizeof(unknown_pdu), resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
}

void test_ble_gatt_notification(void)
{
    SYN_BLE_GATT gatt;
    syn_ble_gatt_init(&gatt, test_gatt_table, 4, NULL);

    uint8_t val = 42;
    uint8_t tx_buf[128];
    uint16_t tx_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_notify(&gatt, 1, 0x0002, &val, 1, tx_buf, &tx_len));
    TEST_ASSERT_TRUE(tx_len > 0);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_HCI_PKT_ACL, tx_buf[0]);

    /* Oversized notification payload error */
    uint8_t huge_val[300];
    uint8_t huge_tx[500];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gatt_notify(&gatt, 1, 0x0002, huge_val,
                                                             sizeof(huge_val), huge_tx, &tx_len));
}

static void check_gatt_permissions_and_cccd(void)
{
    static const uint8_t svc_uuid1[2] = {0x09U, 0x18U};
    static const uint8_t ro_val[2] = {0x12U, 0x34U};
    static const SYN_BLE_GATT_Attr perm_table[] = {
        {0x0001U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc_uuid1, 2U},
        /* Write-only characteristic (has read_cb/static_val, but NO SYN_BLE_PROP_READ) */
        {0x0002U, 0x2A56U, SYN_BLE_PROP_WRITE, temp_read_cb, temp_write_cb, ro_val, 2U},
        /* Write-Cmd-only characteristic (NO SYN_BLE_PROP_WRITE) */
        {0x0003U, 0x2A57U, SYN_BLE_PROP_WRITE_NO_RSP, NULL, temp_write_cb, NULL, 0U},
        /* Read-flag set, but no backing value or callback */
        {0x0004U, 0x2A58U, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        /* Two independent CCCDs supporting Read + Write + WriteCmd */
        {0x0005U, SYN_BLE_UUID_CCCD,
         SYN_BLE_PROP_READ | SYN_BLE_PROP_WRITE | SYN_BLE_PROP_WRITE_CMD, NULL, NULL, NULL, 0U},
        {0x0006U, SYN_BLE_UUID_CCCD,
         SYN_BLE_PROP_READ | SYN_BLE_PROP_WRITE | SYN_BLE_PROP_WRITE_CMD, NULL, NULL, NULL, 0U},
    };

    SYN_BLE_GATT gatt;
    uint8_t resp[64];
    uint16_t resp_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&gatt, perm_table, 6U, NULL));

    /* Invalid PDU lengths */
    uint8_t bad_mtu[] = {SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ, 0x17U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_mtu, 2U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_PDU, resp[4]);

    uint8_t bad_read[] = {SYN_BLE_ATT_OP_READ_REQ, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_read, 2U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_PDU, resp[4]);

    uint8_t bad_write[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_write, 2U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_PDU, resp[4]);

    uint8_t bad_wcmd[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_wcmd, 2U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);

    /* Handle 0x0000 -> INVALID_HANDLE (0x01) on Read Req & Write Req, ignored on Write Cmd */
    uint8_t read_h0[] = {SYN_BLE_ATT_OP_READ_REQ, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, read_h0, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_HANDLE, resp[4]);

    uint8_t write_h0[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x00U, 0x00U, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, write_h0, 4U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_HANDLE, resp[4]);

    uint8_t wcmd_h0[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x00U, 0x00U, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, wcmd_h0, 4U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);

    /* Read on write-only characteristic -> READ_NOT_PERMITTED (0x02) */
    uint8_t read_wo[] = {SYN_BLE_ATT_OP_READ_REQ, 0x02U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, read_wo, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp[4]);

    /* Read on attribute with PROP_READ but no source -> READ_NOT_PERMITTED (0x02) */
    uint8_t read_empty[] = {SYN_BLE_ATT_OP_READ_REQ, 0x04U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, read_empty, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp[4]);

    /* Write Req on Write-Cmd-only characteristic -> WRITE_NOT_PERMITTED (0x03) */
    mock_temp = 11U;
    uint8_t wreq_no_perm[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x03U, 0x00U, 99U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, wreq_no_perm, 4U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED, resp[4]);
    TEST_ASSERT_EQUAL_UINT8(11U, mock_temp);

    /* Write Cmd on Write-Req-only characteristic -> silently ignored without response */
    uint8_t wcmd_no_perm[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x02U, 0x00U, 88U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, wcmd_no_perm, 4U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);
    TEST_ASSERT_EQUAL_UINT8(11U, mock_temp);

    /* CCCD (0x2902) Read, Write Req (valid + invalid len), and Write Cmd */
    uint8_t cccd_bad_len[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x05U, 0x00U, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd_bad_len, 4U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_ATTR_LEN, resp[4]);

    uint8_t cccd_write[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x05U, 0x00U, 0x01U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd_write, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_WRITE_RSP, resp[0]);

    uint8_t cccd_read[] = {SYN_BLE_ATT_OP_READ_REQ, 0x05U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd_read, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(3U, resp_len);
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[1]);

    uint8_t cccd_wcmd[] = {SYN_BLE_ATT_OP_WRITE_CMD, 0x05U, 0x00U, 0x02U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd_wcmd, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0x0002U, gatt.cccd_vals[1U % SYN_BLE_MAX_CONNECTIONS][0]);

    /* Writing second CCCD (0x0006) on same connection must not clobber first CCCD (0x0005) */
    uint8_t cccd2_write[] = {SYN_BLE_ATT_OP_WRITE_REQ, 0x06U, 0x00U, 0x01U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd2_write, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0x0002U, gatt.cccd_vals[1U % SYN_BLE_MAX_CONNECTIONS][0]);
    TEST_ASSERT_EQUAL_UINT16(0x0001U, gatt.cccd_vals[1U % SYN_BLE_MAX_CONNECTIONS][1]);
    uint8_t cccd2_read[] = {SYN_BLE_ATT_OP_READ_REQ, 0x06U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, cccd2_read, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[1]);
}

static void check_gatt_discovery(void)
{
    static const uint8_t svc16_a[2] = {0x0DU, 0x18U};
    static const uint8_t svc16_b[2] = {0x0FU, 0x18U};
    static const uint8_t svc16_c[2] = {0x0AU, 0x18U};
    static const uint8_t svc16_d[2] = {0x00U, 0x18U};
    static const uint8_t svc128[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    static const uint8_t decl_explicit[5] = {SYN_BLE_PROP_READ | SYN_BLE_PROP_NOTIFY, 0x03U, 0x00U,
                                             0x37U, 0x2AU};
    static const uint8_t v2[2] = {0xAAU, 0xBBU};
    static const uint8_t v4[4] = {1U, 2U, 3U, 4U};

    static const SYN_BLE_GATT_Attr disc_table[] = {
        {0x0001U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc16_a, 2U},
        {0x0002U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, decl_explicit, 5U},
        {0x0003U, 0x2A37U, SYN_BLE_PROP_READ | SYN_BLE_PROP_NOTIFY, NULL, NULL, v2, 2U},
        {0x0004U, SYN_BLE_UUID_CCCD, SYN_BLE_PROP_READ | SYN_BLE_PROP_WRITE, NULL, NULL, NULL, 0U},
        /* Synthesized 0x2803 declaration (static_val == NULL, uses next row) */
        {0x0005U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        {0x0006U, 0x2A38U, SYN_BLE_PROP_READ, NULL, NULL, v2, 2U},
        /* Second, third, fourth 16-bit Primary Services + empty service + 128-bit service */
        {0x0010U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        {0x0011U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc16_b, 2U},
        {0x0012U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        {0x0013U, 0x2A19U, SYN_BLE_PROP_READ, NULL, NULL, v2, 2U},
        {0x0014U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        {0x0015U, 0x2A37U, SYN_BLE_PROP_READ, NULL, NULL, v4, 4U}, /* Different val len */
        {0x0020U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc16_c, 2U},
        {0x0030U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc16_d, 2U},
        {0x0040U, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, svc128, 16U},
        /* Dangling trailing 0x2803 without value row */
        {0x0050U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
    };

    SYN_BLE_GATT gatt;
    uint8_t resp[64];
    uint16_t resp_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&gatt, disc_table, 16U, NULL));

    /* 0x10 Read By Group Type: Primary Services (0x2800) */
    uint8_t grp_req[] = {
        SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x00U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, grp_req, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(6U, resp[1]);    /* 4B handles + 2B UUID */
    TEST_ASSERT_EQUAL_UINT16(20U, resp_len); /* 2B hdr + 3 * 6B (MTU cap at 3 services) */
    TEST_ASSERT_EQUAL_UINT8(0x01U, resp[2]); /* svc 1 start = 0x0001 */
    TEST_ASSERT_EQUAL_UINT8(0x06U, resp[4]); /* svc 1 end = 0x0006 */

    /* Group discovery stop on different UUID length (0x0030 is 2B, 0x0040 is 16B) */
    uint8_t grp_req2[] = {
        SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x30U, 0x00U, 0xFFU, 0xFFU, 0x00U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, grp_req2, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(8U, resp_len);

    /* Unsupported group type + not found + invalid range/len */
    uint8_t bad_grp_uuid[] = {
        SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x03U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_grp_uuid, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE, resp[4]);

    uint8_t empty_grp[] = {
        SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x60U, 0x00U, 0x70U, 0x00U, 0x00U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, empty_grp, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    uint8_t bad_grp_len[] = {SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x01U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, bad_grp_len, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_PDU, resp[4]);

    /* 0x08 Read By Type: Characteristic Declarations (0x2803) */
    uint8_t chr_req[] = {SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x03U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, chr_req, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_TYPE_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(7U, resp[1]);    /* 2B decl handle + 5B decl value */
    TEST_ASSERT_EQUAL_UINT16(23U, resp_len); /* 2B hdr + 3 * 7B = 23B MTU */

    /* Trailing dangling 0x2803 at 0x0050 -> ATTR_NOT_FOUND */
    uint8_t chr_dangling[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x50U, 0x00U, 0xFFU, 0xFFU, 0x03U, 0x28U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, chr_dangling, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    /* Compact table without 0x2803 rows returns ATTR_NOT_FOUND instead of illegal decl==val */
    SYN_BLE_GATT compact_gatt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&compact_gatt, test_gatt_table, 4U, NULL));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gatt_process_att_pdu(&compact_gatt, 1U, chr_req, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_ERROR_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    /* Reject malformed 0x2803 declarations where val_handle <= decl_handle + 128-bit UUID PDUs */
    static const uint8_t bad_decl_static[5] = {SYN_BLE_PROP_READ, 0x02U, 0x00U, 0x37U, 0x2AU};
    static const SYN_BLE_GATT_Attr bad_decl_table[] = {
        {0x0002U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, bad_decl_static, 5U},
        {0x0005U, SYN_BLE_UUID_CHARACTERISTIC, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U},
        {0x0004U, 0x2A37U, SYN_BLE_PROP_READ, long_dyn_read_cb, NULL, NULL, 0U},
    };
    SYN_BLE_GATT bad_decl_gatt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&bad_decl_gatt, bad_decl_table, 3U, NULL));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gatt_process_att_pdu(&bad_decl_gatt, 1U, chr_req, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    /* Long dynamic read_cb (>64B) over Read By Type (0x08) & Read Req (0x0A) clamps safely */
    uint8_t rbt_long[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x37U, 0x2AU};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gatt_process_att_pdu(&bad_decl_gatt, 1U, rbt_long, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_TYPE_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(21U, resp[1]); /* 2B handle + 19B clamped value */
    TEST_ASSERT_EQUAL_UINT16(SYN_BLE_ATT_MTU_DEFAULT, resp_len);
    uint8_t rd_long[] = {SYN_BLE_ATT_OP_READ_REQ, 0x04U, 0x00U};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gatt_process_att_pdu(&bad_decl_gatt, 1U, rd_long, 3U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(SYN_BLE_ATT_MTU_DEFAULT, resp_len);

    /* 21-byte 128-bit UUID Read By Type (0x08) & Read By Group Type (0x10) */
    uint8_t uuid128_rbt[21] = {SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, uuid128_rbt, 21U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    uint8_t uuid128_grp[21] = {SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, uuid128_grp, 21U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE, resp[4]);

    uint8_t rbt_unreadable[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x00U, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&compact_gatt, 1U, rbt_unreadable, 7U,
                                                           resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_READ_NOT_PERMITTED, resp[4]);

    uint8_t rbt_val[] = {SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x37U, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, rbt_val, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_READ_BY_TYPE_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(4U, resp[1]); /* 2B handle + 2B v2 (stops before 4B v4) */

    uint8_t rbt_inv_range[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x10U, 0x00U, 0x05U, 0x00U, 0x37U, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, rbt_inv_range, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_HANDLE, resp[4]);

    /* Read By Type: not found, zero-len skip, MTU break, and unreadable-after-readable break */
    static const uint8_t v8[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    static const SYN_BLE_GATT_Attr rbt_edge_table[] = {
        {0x0001U, 0x2A99U, SYN_BLE_PROP_READ, NULL, NULL, NULL, 0U}, /* unreadable/empty -> skip */
        {0x0002U, 0x2A99U, SYN_BLE_PROP_READ, NULL, NULL, v8, 8U},   /* pair_len = 10 */
        {0x0003U, 0x2A99U, SYN_BLE_PROP_READ, NULL, NULL, v8, 8U},   /* list_len = 20 */
        {0x0004U, 0x2A99U, SYN_BLE_PROP_READ, NULL, NULL, v8, 8U},   /* 30 > 21 -> MTU break */
        {0x0005U, 0x2A9AU, SYN_BLE_PROP_READ, NULL, NULL, v2, 2U},
        {0x0006U, 0x2A9AU, 0U, NULL, NULL, v2, 2U}, /* unreadable 2nd match -> break */
    };
    SYN_BLE_GATT rbt_gatt;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&rbt_gatt, rbt_edge_table, 6U, NULL));
    uint8_t rbt_mtu[] = {SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x99U, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&rbt_gatt, 1U, rbt_mtu, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(22U, resp_len);

    uint8_t rbt_stop[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0x9AU, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&rbt_gatt, 1U, rbt_stop, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT16(6U, resp_len);

    uint8_t rbt_miss[] = {
        SYN_BLE_ATT_OP_READ_BY_TYPE_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU, 0xFEU, 0x2AU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&rbt_gatt, 1U, rbt_miss, 7U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    /* 0x04 Find Information (handles + CCCD 0x2902) */
    uint8_t fi_req[] = {SYN_BLE_ATT_OP_FIND_INFO_REQ, 0x01U, 0x00U, 0xFFU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1U, fi_req, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_OP_FIND_INFO_RSP, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_FIND_INFO_FMT_UUID16, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(22U, resp_len); /* 2B hdr + 5 * 4B pairs (includes 0x0004 -> 0x2902) */
    TEST_ASSERT_EQUAL_UINT8(0x02U, resp[16]); /* 4th pair UUID = 0x2902 LSB */
    TEST_ASSERT_EQUAL_UINT8(0x29U, resp[17]); /* 4th pair UUID = 0x2902 MSB */

    uint8_t fi_empty[] = {SYN_BLE_ATT_OP_FIND_INFO_REQ, 0x60U, 0x00U, 0x70U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gatt_process_att_pdu(&gatt, 1U, fi_empty, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_ATTR_NOT_FOUND, resp[4]);

    uint8_t fi_inv[] = {SYN_BLE_ATT_OP_FIND_INFO_REQ, 0x00U, 0x00U, 0x10U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_process_att_pdu(&gatt, 1U, fi_inv, 5U, resp, &resp_len));
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_ATT_ERR_INVALID_HANDLE, resp[4]);
}

void test_r5_ble_gatt_regressions(void)
{
    check_gatt_permissions_and_cccd();
    check_gatt_discovery();

    /* BLE-1a: Responses/ErrorRsp (0x01, 0x03, 0x1B) and Confirmations (0x1E) must NOT
     * emit ATT_ERROR_RSP */
    SYN_BLE_GATT gatt;
    uint8_t resp[64];
    uint16_t resp_len = 99U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_init(&gatt, test_gatt_table, 4U, NULL));

    static const uint8_t silent_ops[4] = {0x01U, 0x03U, 0x1BU, 0x1EU};
    for (size_t i = 0U; i < 4U; i++) {
        uint8_t pdu[3] = {silent_ops[i], 0x00U, 0x00U};
        resp_len = 99U;
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_ble_gatt_process_att_pdu(&gatt, 1U, pdu, 3U, resp, &resp_len));
        TEST_ASSERT_EQUAL_UINT16(0U, resp_len);
    }

    /* BLE-4: Notify enforces ATT_MTU - 3 (20B) and rejects NULL val with non-zero len */
    uint8_t val20[21] = {0};
    uint8_t tx_buf[64];
    uint16_t tx_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gatt_notify(&gatt, 1U, 0x0002U, val20, 20U, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_gatt_notify(&gatt, 1U, 0x0002U, val20, 21U, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ble_gatt_notify(&gatt, 1U, 0x0002U, NULL, 5U, tx_buf, &tx_len));
}
