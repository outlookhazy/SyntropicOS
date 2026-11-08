#include "syntropic/ble/syn_ble_hci.h"
#include "unity/unity.h"

#include <string.h>

static uint8_t last_evt_code;
static uint8_t last_evt_payload[64];
static uint8_t last_evt_len;
static bool evt_called;

static uint16_t last_acl_conn;
static uint8_t last_acl_flags;
static uint8_t last_acl_data[64];
static uint16_t last_acl_len;
static bool acl_called;

static void test_evt_cb(SYN_BLE_HCI *hci, uint8_t evt_code, const uint8_t *payload, uint8_t len,
                        void *user_data)
{
    (void)hci;
    (void)user_data;
    last_evt_code = evt_code;
    last_evt_len = len;
    if (payload != NULL && len <= sizeof(last_evt_payload)) {
        memcpy(last_evt_payload, payload, len);
    }
    evt_called = true;
}

static void test_acl_cb(SYN_BLE_HCI *hci, uint16_t conn_handle, uint8_t pb_bc_flags,
                        const uint8_t *data, uint16_t len, void *user_data)
{
    (void)hci;
    (void)user_data;
    last_acl_conn = conn_handle;
    last_acl_flags = pb_bc_flags;
    last_acl_len = len;
    if (data != NULL && len <= sizeof(last_acl_data)) {
        memcpy(last_acl_data, data, len);
    }
    acl_called = true;
}

void test_ble_hci_null_params(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_init(NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_rx_byte(NULL, 0x00));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_rx_buf(NULL, NULL, 0));

    uint8_t buf[16];
    uint16_t len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_encode_command(0x0000, NULL, 0, NULL, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_encode_acl(0x0001, 0, 0, NULL, 5, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_hci_encode_command(0x0001, NULL, 5, buf, &len));
}

void test_ble_hci_encode_command(void)
{
    uint8_t buf[32];
    uint16_t len = 0;
    uint8_t params[] = {0x01, 0x02, 0x03};

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_hci_encode_command(SYN_BLE_HCI_OP_LE_SET_ADV_ENABLE, params, 3, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(7, len);

    /* Zero params */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_hci_encode_command(SYN_BLE_HCI_OP_LE_READ_BUFFER_SIZE, NULL, 0, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(4, len);
}

void test_ble_hci_encode_acl(void)
{
    uint8_t buf[32];
    uint16_t len = 0;
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_encode_acl(0x0005, 0x02, 0x00, payload, 3, buf, &len));
    TEST_ASSERT_EQUAL_UINT16(8, len);
}

void test_ble_hci_rx_event_dispatch(void)
{
    SYN_BLE_HCI hci;
    SYN_BLE_HCI_Config cfg = {.evt_cb = test_evt_cb, .acl_cb = test_acl_cb, .user_data = NULL};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_init(&hci, &cfg));

    /* Command Complete Event */
    evt_called = false;
    uint8_t cmd_complete_stream[] = {
        SYN_BLE_HCI_PKT_EVT, SYN_BLE_HCI_EVT_CMD_COMPLETE, 0x04, 0x01, 0x03, 0x0C, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_hci_rx_buf(&hci, cmd_complete_stream, sizeof(cmd_complete_stream)));
    TEST_ASSERT_TRUE(evt_called);
    TEST_ASSERT_EQUAL_UINT16(0x0C03, hci.last_cmd_opcode);
    TEST_ASSERT_EQUAL_UINT8(0x00, hci.last_cmd_status);

    /* Disconnect Complete Event */
    evt_called = false;
    uint8_t evt_stream[] = {
        SYN_BLE_HCI_PKT_EVT, SYN_BLE_HCI_EVT_DISCONN_COMPLETE, 0x04, 0x00, 0x01, 0x00, 0x13};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, evt_stream, sizeof(evt_stream)));
    TEST_ASSERT_TRUE(evt_called);

    /* Command Status Event */
    evt_called = false;
    uint8_t status_stream[] = {
        SYN_BLE_HCI_PKT_EVT, SYN_BLE_HCI_EVT_CMD_STATUS, 0x04, 0x00, 0x01, 0x0A, 0x20};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, status_stream, sizeof(status_stream)));
    TEST_ASSERT_TRUE(evt_called);
    TEST_ASSERT_EQUAL_UINT8(0x00, hci.last_cmd_status);

    /* Zero payload event (dispatches directly in header state) */
    evt_called = false;
    uint8_t zero_payload_evt[] = {SYN_BLE_HCI_PKT_EVT, 0x05, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, zero_payload_evt, sizeof(zero_payload_evt)));
    TEST_ASSERT_TRUE(evt_called);
}

void test_ble_hci_rx_acl_dispatch(void)
{
    SYN_BLE_HCI hci;
    SYN_BLE_HCI_Config cfg = {.evt_cb = test_evt_cb, .acl_cb = test_acl_cb, .user_data = NULL};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_init(&hci, &cfg));

    acl_called = false;
    uint8_t acl_stream[] = {SYN_BLE_HCI_PKT_ACL, 0x02, 0x20, 0x03, 0x00, 0x11, 0x22, 0x33};

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, acl_stream, sizeof(acl_stream)));
    TEST_ASSERT_TRUE(acl_called);
    TEST_ASSERT_EQUAL_UINT16(0x0002, last_acl_conn);
}

void test_ble_hci_rx_overflow_and_invalid(void)
{
    SYN_BLE_HCI hci;
    syn_ble_hci_init(&hci, NULL);

    /* Invalid Indicator Byte */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_byte(&hci, 0xFF));

    /* ACL payload size exceeding buffer (len = 257 B -> 4 + 257 = 261 B > 260 B) */
    uint8_t huge_acl_hdr[] = {SYN_BLE_HCI_PKT_ACL, 0x01, 0x00, 0x01, 0x01};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_hci_rx_buf(&hci, huge_acl_hdr, sizeof(huge_acl_hdr)));

    /* Max-length 255B HCI Event (2B hdr + 255B params = 257B <= 260B) succeeds cleanly */
    uint8_t max_evt[258];
    memset(max_evt, 0x5AU, sizeof(max_evt));
    max_evt[0] = SYN_BLE_HCI_PKT_EVT;
    max_evt[1] = 0x3EU;
    max_evt[2] = 0xFFU;
    syn_ble_hci_init(&hci, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, max_evt, sizeof(max_evt)));

    /* Payload state overflow */
    syn_ble_hci_init(&hci, NULL);
    hci.rx_state = 2; /* PAYLOAD state */
    hci.rx_idx = SYN_BLE_HCI_RX_BUF_SIZE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_hci_rx_byte(&hci, 0xAA));

    /* Header state overflow */
    syn_ble_hci_init(&hci, NULL);
    hci.rx_state = 1; /* HDR state */
    hci.rx_idx = SYN_BLE_HCI_RX_BUF_SIZE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_hci_rx_byte(&hci, 0xAA));

    /* Invalid state machine enum value (triggers default branch) */
    syn_ble_hci_init(&hci, NULL);
    hci.rx_state = 99;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_byte(&hci, 0x00));
}

void test_ble_hci_cmd_complete_short_params(void)
{
    SYN_BLE_HCI hci;
    SYN_BLE_HCI_Config cfg = {.evt_cb = test_evt_cb, .acl_cb = NULL, .user_data = NULL};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_init(&hci, &cfg));

    /* Poison last_cmd_status to detect whether it gets safely set */
    hci.last_cmd_status = 0xFFU;

    /* Command Complete with param_len=3: Num_HCI_Command_Packets(1) + Opcode(2), no return params.
     * Pre-fix: guard is param_len >= 3U but code reads payload[3] → 1 byte OOB. */
    evt_called = false;
    uint8_t short_cc[] = {
        SYN_BLE_HCI_PKT_EVT, SYN_BLE_HCI_EVT_CMD_COMPLETE, 0x03, 0x01, 0x01, 0x0C};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, short_cc, sizeof(short_cc)));
    TEST_ASSERT_TRUE(evt_called);
    TEST_ASSERT_EQUAL_UINT16(0x0C01, hci.last_cmd_opcode);
    /* Status must default to 0 when no return params present, not garbage from OOB read */
    TEST_ASSERT_EQUAL_UINT8(0x00, hci.last_cmd_status);
}

void test_r5_ble_hci_regressions(void)
{
    SYN_BLE_HCI hci;
    SYN_BLE_HCI_Config cfg = {.evt_cb = test_evt_cb, .acl_cb = test_acl_cb, .user_data = NULL};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_init(&hci, &cfg));

    /* 16-bit wraparound attack: 4 + 0xFFFC == 0x10000 (wrapped to 0 in uint16_t pre-fix) */
    acl_called = false;
    uint8_t wrap_acl_hdr[] = {SYN_BLE_HCI_PKT_ACL, 0x01U, 0x20U, 0xFCU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_hci_rx_buf(&hci, wrap_acl_hdr, sizeof(wrap_acl_hdr)));
    TEST_ASSERT_FALSE(acl_called);

    /* 0xFFFF ACL length also rejected cleanly */
    uint8_t max_acl_hdr[] = {SYN_BLE_HCI_PKT_ACL, 0x01U, 0x20U, 0xFFU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_hci_rx_buf(&hci, max_acl_hdr, sizeof(max_acl_hdr)));
    TEST_ASSERT_FALSE(acl_called);

    /* Subsequent valid ACL frame resynchronizes cleanly */
    uint8_t valid_acl[] = {SYN_BLE_HCI_PKT_ACL, 0x05U, 0x20U, 0x02U, 0x00U, 0xABU, 0xCDU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_hci_rx_buf(&hci, valid_acl, sizeof(valid_acl)));
    TEST_ASSERT_TRUE(acl_called);
    TEST_ASSERT_EQUAL_UINT16(0x0005U, last_acl_conn);
    TEST_ASSERT_EQUAL_UINT16(2U, last_acl_len);
}
