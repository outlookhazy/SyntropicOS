#include "syntropic/ble/syn_ble_gap.h"
#include "unity/unity.h"

#include <string.h>

static SYN_BLE_GAP_EventType last_gap_evt;
static bool gap_evt_called;
static int8_t last_rssi;

static void test_gap_cb(SYN_BLE_GAP *gap, SYN_BLE_GAP_EventType evt_type, const void *evt_data,
                        void *user_data)
{
    (void)gap;
    (void)user_data;
    last_gap_evt = evt_type;
    gap_evt_called = true;
    if (evt_type == SYN_BLE_GAP_EVT_ADV_REPORT && evt_data != NULL) {
        const SYN_BLE_GAP_AdvReport *rep = (const SYN_BLE_GAP_AdvReport *)evt_data;
        last_rssi = rep->rssi;
    }
}

void test_ble_gap_null_params(void)
{
    SYN_BLE_HCI hci;
    syn_ble_hci_init(&hci, NULL);

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_init(NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_init(NULL, &hci, NULL, NULL));

    SYN_BLE_GAP gap;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_init(&gap, &hci, NULL, NULL));

    uint8_t buf[64];
    uint16_t len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_set_adv_data(NULL, NULL, 0, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_set_adv_data(&gap, buf, 32, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_set_adv_enable(NULL, true, buf, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_process_hci_evt(NULL, 0, NULL, 0));
}

void test_ble_gap_adv_data_encoding(void)
{
    SYN_BLE_HCI hci;
    syn_ble_hci_init(&hci, NULL);
    SYN_BLE_GAP gap;
    syn_ble_gap_init(&gap, &hci, test_gap_cb, NULL);

    uint8_t adv_payload[] = {0x02, 0x01, 0x06, 0x05, 0x09, 'T', 'e', 's', 't'};
    uint8_t tx_buf[64];
    uint16_t tx_len = 0;

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ble_gap_set_adv_data(&gap, adv_payload, sizeof(adv_payload), tx_buf, &tx_len));
    TEST_ASSERT_EQUAL_UINT16(36, tx_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_BLE_HCI_PKT_CMD, tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x08, tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0x20, tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(32, tx_buf[3]);
    TEST_ASSERT_EQUAL_UINT8(sizeof(adv_payload), tx_buf[4]);
    TEST_ASSERT_EQUAL_UINT8('T', tx_buf[10]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_set_adv_enable(&gap, true, tx_buf, &tx_len));
    TEST_ASSERT_TRUE(gap.advertising);
    TEST_ASSERT_EQUAL_UINT8(0x01, tx_buf[4]);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_set_adv_enable(&gap, false, tx_buf, &tx_len));
    TEST_ASSERT_FALSE(gap.advertising);
    TEST_ASSERT_EQUAL_UINT8(0x00, tx_buf[4]);
}

void test_ble_gap_process_events(void)
{
    SYN_BLE_HCI hci;
    syn_ble_hci_init(&hci, NULL);
    SYN_BLE_GAP gap;
    syn_ble_gap_init(&gap, &hci, test_gap_cb, NULL);
    uint8_t tx_buf[64];
    uint16_t tx_len = 0;

    /* Disconnect Complete Event */
    gap_evt_called = false;
    uint8_t disconn_evt[] = {0x00, 0x01, 0x00, 0x13};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_DISCONN_COMPLETE,
                                                          disconn_evt, sizeof(disconn_evt)));
    TEST_ASSERT_TRUE(gap_evt_called);
    TEST_ASSERT_EQUAL(SYN_BLE_GAP_EVT_DISCONNECTED, last_gap_evt);

    /* LE Connection Complete Meta Event */
    gap_evt_called = false;
    uint8_t conn_complete_evt[] = {SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE,
                                   0x00,
                                   0x01,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0xAA,
                                   0xBB,
                                   0xCC,
                                   0xDD,
                                   0xEE,
                                   0xFF,
                                   0x18,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x40,
                                   0x00,
                                   0x00};

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, conn_complete_evt,
                                                  sizeof(conn_complete_evt)));
    TEST_ASSERT_TRUE(gap_evt_called);
    TEST_ASSERT_EQUAL(SYN_BLE_GAP_EVT_CONNECTED, last_gap_evt);

    /* LE Advertising Report Meta Event with RSSI */
    gap_evt_called = false;
    last_rssi = 0;
    uint8_t adv_report_evt[] = {
        SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
        0x01, /* num_reports */
        0x00, /* evt_type */
        0x00, /* addr_type */
        0x11,
        0x22,
        0x33,
        0x44,
        0x55,
        0x66,
        0x03, /* data_len */
        0x02,
        0x01,
        0x06,
        0xCE /* rssi (-50) */
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                          adv_report_evt, sizeof(adv_report_evt)));
    TEST_ASSERT_TRUE(gap_evt_called);
    TEST_ASSERT_EQUAL_INT8(-50, last_rssi);

    /* Advertising Report without trailing RSSI byte -> triggers fallback rssi=0 */
    gap_evt_called = false;
    last_rssi = -50;
    uint8_t no_rssi_adv_report[] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
                                    0x01,
                                    0x00,
                                    0x00,
                                    0x11,
                                    0x22,
                                    0x33,
                                    0x44,
                                    0x55,
                                    0x66,
                                    0x03, /* data_len = 3, len = 14 (exact AD payload, no RSSI) */
                                    0x02,
                                    0x01,
                                    0x06};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, no_rssi_adv_report,
                                                  sizeof(no_rssi_adv_report)));
    TEST_ASSERT_TRUE(gap_evt_called);
    TEST_ASSERT_EQUAL_INT8(0, last_rssi);

    /* Unknown event code with non-null payload */
    uint8_t dummy_evt[] = {0x00};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap, 0xFF, dummy_evt, sizeof(dummy_evt)));

    /* 0-length advertising data */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_set_adv_data(&gap, NULL, 0, tx_buf, &tx_len));

    /* Non-zero status error codes in HCI events */
    gap_evt_called = false;
    uint8_t disconn_fail[] = {0x05, 0x01, 0x00, 0x13};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_DISCONN_COMPLETE,
                                                          disconn_fail, sizeof(disconn_fail)));
    TEST_ASSERT_FALSE(gap_evt_called);

    uint8_t conn_fail[] = {SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE,
                           0x02,
                           0x01,
                           0x00,
                           0x00,
                           0x00,
                           0xAA,
                           0xBB,
                           0xCC,
                           0xDD,
                           0xEE,
                           0xFF,
                           0x18,
                           0x00,
                           0x00,
                           0x00,
                           0x40,
                           0x00,
                           0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, conn_fail,
                                                          sizeof(conn_fail)));
    TEST_ASSERT_FALSE(gap_evt_called);

    /* Zero reports in advertising report event */
    uint8_t zero_adv_report[] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, zero_adv_report,
                                                  sizeof(zero_adv_report)));
    TEST_ASSERT_FALSE(gap_evt_called);

    /* Events with NULL callback */
    SYN_BLE_GAP gap_nocb;
    syn_ble_gap_init(&gap_nocb, &hci, NULL, NULL);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap_nocb, SYN_BLE_HCI_EVT_DISCONN_COMPLETE,
                                                  disconn_evt, sizeof(disconn_evt)));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ble_gap_process_hci_evt(&gap_nocb, SYN_BLE_HCI_EVT_LE_META,
                                                  conn_complete_evt, sizeof(conn_complete_evt)));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap_nocb, SYN_BLE_HCI_EVT_LE_META,
                                                          adv_report_evt, sizeof(adv_report_evt)));
}

void test_r5_ble_gap_regressions(void)
{
    SYN_BLE_HCI hci;
    syn_ble_hci_init(&hci, NULL);
    SYN_BLE_GAP gap;
    syn_ble_gap_init(&gap, &hci, test_gap_cb, NULL);

    /* len == 10 (< 11 minimum to read data_len at payload[10]) -> rejected without OOB read */
    gap_evt_called = false;
    uint8_t short_10b[10] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
                             0x01U,
                             0x00U,
                             0x00U,
                             0x11U,
                             0x22U,
                             0x33U,
                             0x44U,
                             0x55U,
                             0x66U};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                             short_10b, sizeof(short_10b)));
    TEST_ASSERT_FALSE(gap_evt_called);

    /* 11 + data_len > len -> rejected without OOB read */
    uint8_t trunc_payload[13] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
                                 0x01U,
                                 0x00U,
                                 0x00U,
                                 0x11U,
                                 0x22U,
                                 0x33U,
                                 0x44U,
                                 0x55U,
                                 0x66U,
                                 0x03U,
                                 0x02U,
                                 0x01U};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                             trunc_payload, sizeof(trunc_payload)));
    TEST_ASSERT_FALSE(gap_evt_called);

    /* data_len > 31 (legacy BLE AD limit) -> rejected */
    uint8_t over_ad[44];
    memset(over_ad, 0, sizeof(over_ad));
    over_ad[0] = SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT;
    over_ad[1] = 0x01U;
    over_ad[10] = 32U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, over_ad,
                                                             sizeof(over_ad)));
    TEST_ASSERT_FALSE(gap_evt_called);

    /* Exact 11-byte empty-data advertisement (data_len == 0, no RSSI byte) */
    uint8_t empty_ad[11] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
                            0x01U,
                            0x00U,
                            0x00U,
                            0x11U,
                            0x22U,
                            0x33U,
                            0x44U,
                            0x55U,
                            0x66U,
                            0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, empty_ad,
                                                          sizeof(empty_ad)));
    TEST_ASSERT_TRUE(gap_evt_called);
    TEST_ASSERT_EQUAL_INT8(0, last_rssi);

    /* BLE2-2: 1-byte truncated adv report (< 2B) + Num_Reports > 25 + striped 2-report batch */
    uint8_t one_byte[1] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                             one_byte, sizeof(one_byte)));

    uint8_t too_many[4] = {SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT, 26U, 0U, 0U};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                             too_many, sizeof(too_many)));

    uint8_t multi_rep[] = {
        SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT,
        0x02U, /* Num_Reports = 2 */
        0x00U,
        0x03U, /* Event_Type[2] */
        0x00U,
        0x01U, /* Address_Type[2] */
        0xA1U,
        0xA2U,
        0xA3U,
        0xA4U,
        0xA5U,
        0xA6U, /* Address[0] */
        0xB1U,
        0xB2U,
        0xB3U,
        0xB4U,
        0xB5U,
        0xB6U, /* Address[1] */
        0x02U,
        0x01U, /* Data_Length[2] = {2, 1} */
        0xAAU,
        0xBBU, /* Data[0] */
        0xCCU, /* Data[1] */
        (uint8_t)-42,
        (uint8_t)-67 /* RSSI[2] */
    };
    last_rssi = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META, multi_rep,
                                                          sizeof(multi_rep)));
    TEST_ASSERT_EQUAL_INT8(-67, last_rssi);

    /* BLE2-4: Reject NULL adv data with len > 0, and clear advertising on Peripheral connect */
    uint8_t tx_buf[64];
    uint16_t tx_len = 0U;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ble_gap_set_adv_data(&gap, NULL, 4U, tx_buf, &tx_len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_set_adv_enable(&gap, true, tx_buf, &tx_len));
    TEST_ASSERT_TRUE(gap.advertising);

    uint8_t periph_conn[19] = {SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE,
                               0x00U,
                               0x01U,
                               0x00U,
                               0x01U, /* Role = 0x01 (Peripheral) */
                               0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ble_gap_process_hci_evt(&gap, SYN_BLE_HCI_EVT_LE_META,
                                                          periph_conn, sizeof(periph_conn)));
    TEST_ASSERT_FALSE(gap.advertising);
}
