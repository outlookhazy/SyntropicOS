#include "syntropic/ble/syn_ble_l2cap.h"
#include "unity/unity.h"

#include <string.h>

static uint16_t last_l2cap_conn;
static uint16_t last_l2cap_cid;
static uint8_t last_l2cap_payload[256];
static uint16_t last_l2cap_len;
static bool l2cap_rx_called;

static void test_l2cap_cb(SYN_BLE_L2CAP *l2cap, uint16_t conn_handle, uint16_t cid,
                          const uint8_t *payload, uint16_t len, void *user_data)
{
    (void)l2cap;
    (void)user_data;
    last_l2cap_conn = conn_handle;
    last_l2cap_cid = cid;
    last_l2cap_len = len;
    if (payload != NULL && len <= sizeof(last_l2cap_payload)) {
        memcpy(last_l2cap_payload, payload, len);
    }
    l2cap_rx_called = true;
}

void test_ble_l2cap_null_and_init(void)
{
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_init(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_connect(NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_disconnect(NULL, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_process_acl(NULL, 1, 0, NULL, 0));

    uint16_t len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_encode_pdu(1, 4, NULL, 0, NULL, &len));

    SYN_BLE_L2CAP l2cap;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_init(&l2cap, test_l2cap_cb, NULL));
}

void test_ble_l2cap_connect_disconnect_pool(void)
{
    SYN_BLE_L2CAP l2cap;
    syn_ble_l2cap_init(&l2cap, test_l2cap_cb, NULL);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_connect(&l2cap, 0x0001));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_connect(&l2cap, 0x0001)); /* idempotent */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_connect(&l2cap, 0x0002));

    /* Pool full for MAX_CONNECTIONS=2 */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_connect(&l2cap, 0x0003));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_disconnect(&l2cap, 0x0001));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_disconnect(&l2cap, 0x0099)); /* unmapped handle */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_connect(&l2cap, 0x0003));    /* now works */
}

void test_ble_l2cap_single_and_fragmented_acl(void)
{
    SYN_BLE_L2CAP l2cap;
    syn_ble_l2cap_init(&l2cap, test_l2cap_cb, NULL);

    /* Single complete PDU */
    l2cap_rx_called = false;
    uint8_t acl_single[] = {0x03, 0x00, /* len = 3 */
                            0x04, 0x00, /* CID = 4 (ATT) */
                            0x11, 0x22, 0x33};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x02, acl_single, sizeof(acl_single)));
    TEST_ASSERT_TRUE(l2cap_rx_called);
    TEST_ASSERT_EQUAL_UINT16(0x0001, last_l2cap_conn);
    TEST_ASSERT_EQUAL_UINT16(0x0004, last_l2cap_cid);
    TEST_ASSERT_EQUAL_UINT16(3, last_l2cap_len);

    /* Fragmented PDU */
    l2cap_rx_called = false;
    uint8_t acl_frag1[] = {
        0x04, 0x00, /* total len = 4 */
        0x04, 0x00, /* CID = 4 */
        0xAA, 0xBB  /* first 2 bytes */
    };
    uint8_t acl_frag2[] = {
        0xCC, 0xDD /* remaining 2 bytes */
    };

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x00, acl_frag1, sizeof(acl_frag1)));
    TEST_ASSERT_FALSE(l2cap_rx_called);

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x01, acl_frag2, sizeof(acl_frag2)));
    TEST_ASSERT_TRUE(l2cap_rx_called);
    TEST_ASSERT_EQUAL_UINT16(4, last_l2cap_len);
    TEST_ASSERT_EQUAL_UINT8(0xAA, last_l2cap_payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0xDD, last_l2cap_payload[3]);
}

void test_ble_l2cap_errors_and_edge_cases(void)
{
    SYN_BLE_L2CAP l2cap;
    syn_ble_l2cap_init(&l2cap, test_l2cap_cb, NULL);

    /* Short ACL payload (<4B) */
    uint8_t short_acl[] = {0x01, 0x00};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x00, short_acl,
                                                                   sizeof(short_acl)));

    /* Oversized PDU expected len */
    uint8_t huge_pdu_hdr[] = {0xFF, 0x01, 0x04, 0x00}; /* len = 511B > 256B */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x00, huge_pdu_hdr,
                                                           sizeof(huge_pdu_hdr)));

    /* Unexpected continuation packet */
    uint8_t unexpected_cont[] = {0x01, 0x02};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x01, unexpected_cont,
                                                           sizeof(unexpected_cont)));

    /* Continuation packet overflow */
    uint8_t frag1[] = {0xF0, 0x00, 0x04, 0x00, 0x01, 0x02}; /* expected 240 B */
    syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x00, frag1, sizeof(frag1));
    uint8_t overflow_cont[256]; /* 256 + 2 > 256 B rx_buf */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001, 0x01, overflow_cont,
                                                           sizeof(overflow_cont)));

    /* Pool full auto-connect error */
    syn_ble_l2cap_connect(&l2cap, 0x0001);
    syn_ble_l2cap_connect(&l2cap, 0x0002);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0003, 0x00, short_acl, 4));

    /* Encode PDU overflow */
    uint8_t huge_payload[300];
    uint8_t tx_buf[500];
    uint16_t tx_len = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_encode_pdu(0x0001, 4, huge_payload,
                                                          sizeof(huge_payload), tx_buf, &tx_len));
}

void test_ble_l2cap_encode_pdu(void)
{
    uint8_t payload[] = {0x01, 0x02, 0x03};
    uint8_t tx_buf[64];
    uint16_t tx_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_encode_pdu(0x0001, SYN_BLE_L2CAP_CID_ATT, payload, 3,
                                                       tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_UINT16(12, tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_HCI_PKT_ACL, tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x01, tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(7, tx_buf[3]); /* 4B L2CAP hdr + 3B payload */
    TEST_ASSERT_EQUAL_UINT8(0, tx_buf[4]);
    TEST_ASSERT_EQUAL_UINT8(3, tx_buf[5]); /* L2CAP len LSB */
    TEST_ASSERT_EQUAL_UINT8(4, tx_buf[7]); /* L2CAP CID LSB */
}

void test_r5_ble_l2cap_regressions(void)
{
    SYN_BLE_L2CAP l2cap;
    syn_ble_l2cap_init(&l2cap, test_l2cap_cb, NULL);

    /* Single frame with pdu_len > SYN_BLE_L2CAP_BUF_SIZE (300 > 256) must fail */
    uint8_t big_single[304];
    memset(big_single, 0x5AU, sizeof(big_single));
    big_single[0] = 0x2CU; /* 300 LSB */
    big_single[1] = 0x01U; /* 300 MSB */
    big_single[2] = 0x04U;
    big_single[3] = 0x00U;
    l2cap_rx_called = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001U, 0x02U, big_single,
                                                           sizeof(big_single)));
    TEST_ASSERT_FALSE(l2cap_rx_called);

    /* Start frame where ACL payload_len (5) > L2CAP pdu_len (2) must fail */
    uint8_t extra_start[] = {0x02U, 0x00U, 0x04U, 0x00U, 1U, 2U, 3U, 4U, 5U};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001U, 0x02U, extra_start,
                                                           sizeof(extra_start)));
    TEST_ASSERT_FALSE(l2cap_rx_called);

    /* Continuation fragment exceeding declared pdu_len (2 + 3 = 5 > 4) must drop without dispatch
     */
    uint8_t frag1[] = {0x04U, 0x00U, 0x04U, 0x00U, 0xAAU, 0xBBU};
    uint8_t frag2_over[] = {0xCCU, 0xDDU, 0xEEU};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_process_acl(&l2cap, 0x0001U, 0x02U, frag1, sizeof(frag1)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_l2cap_process_acl(&l2cap, 0x0001U, 0x01U, frag2_over,
                                                           sizeof(frag2_over)));
    TEST_ASSERT_FALSE(l2cap_rx_called);

    /* Multi-fragment reassembly (3 fragments) + NULL rx_cb + reserved PB=0x03 */
    SYN_BLE_L2CAP l2cap_nocb;
    syn_ble_l2cap_init(&l2cap_nocb, NULL, NULL);
    uint8_t single_ok[] = {0x01U, 0x00U, 0x04U, 0x00U, 0x99U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_l2cap_process_acl(&l2cap_nocb, 0x0001U, 0x02U, single_ok,
                                                        sizeof(single_ok)));
    uint8_t f1[] = {0x04U, 0x00U, 0x04U, 0x00U, 0x01U};
    uint8_t f2[] = {0x02U};
    uint8_t f3[] = {0x03U, 0x04U};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_process_acl(&l2cap_nocb, 0x0001U, 0x02U, f1, sizeof(f1)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_process_acl(&l2cap_nocb, 0x0001U, 0x01U, f2, sizeof(f2)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_process_acl(&l2cap_nocb, 0x0001U, 0x01U, f3, sizeof(f3)));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_l2cap_process_acl(&l2cap_nocb, 0x0001U, 0x03U,
                                                                   single_ok, sizeof(single_ok)));

    uint8_t tx_buf[16];
    uint16_t tx_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_encode_pdu(0x0001U, 0x0004U, NULL, 0U, tx_buf, &tx_len));

    /* BLE2-1: Full 252B L2CAP payload fits SYN_BLE_HCI_TX_BUF_SIZE (261B), 253B rejected */
    uint8_t max_pay[253] = {0};
    uint8_t hci_tx[SYN_BLE_HCI_TX_BUF_SIZE];
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_l2cap_encode_pdu(0x0001U, 0x0004U, max_pay, 252U, hci_tx, &tx_len));
    TEST_ASSERT_EQUAL_UINT16(SYN_BLE_HCI_TX_BUF_SIZE, tx_len);
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ble_l2cap_encode_pdu(0x0001U, 0x0004U, max_pay, 253U, hci_tx, &tx_len));
}
