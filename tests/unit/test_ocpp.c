/**
 * @file test_ocpp.c
 * @brief Unit tests for Open Charge Point Protocol (OCPP-J 1.6 / 2.0.1) Dual-Role Engine.
 */

#include "syntropic/proto/syn_ocpp.h"
#include "unity/unity.h"

#include <stdio.h>
#include <string.h>

static SYN_OCPP_Client g_ocpp_client;
static SYN_OCPP_Server g_ocpp_server;
static bool g_reg_cb_called = false;
static bool g_auth_cb_called = false;
static bool g_start_tx_cb_called = false;
static bool g_remote_start_called = false;
static bool g_remote_stop_called = false;
static bool g_server_boot_called = false;

static void on_ocpp_reg(SYN_OCPP_RegistrationStatus status, uint32_t interval, void *ctx)
{
    (void)status;
    (void)interval;
    (void)ctx;
    g_reg_cb_called = true;
}

static void on_ocpp_auth(const char *id_tag, SYN_OCPP_AuthorizationStatus status, void *ctx)
{
    (void)id_tag;
    (void)status;
    (void)ctx;
    g_auth_cb_called = true;
}

static void on_ocpp_start_tx(int32_t tx_id, SYN_OCPP_AuthorizationStatus status, void *ctx)
{
    (void)tx_id;
    (void)status;
    (void)ctx;
    g_start_tx_cb_called = true;
}

static bool on_ocpp_remote_start(uint32_t conn_id, const char *id_tag, void *ctx)
{
    (void)conn_id;
    (void)id_tag;
    (void)ctx;
    g_remote_start_called = true;
    return true;
}

static bool on_ocpp_remote_stop(int32_t tx_id, void *ctx)
{
    (void)tx_id;
    (void)ctx;
    g_remote_stop_called = true;
    return true;
}

static SYN_OCPP_RegistrationStatus on_server_boot(const SYN_OCPP_ChargePointInfo *info,
                                                  uint32_t *hb_sec, void *ctx)
{
    (void)info;
    (void)ctx;
    g_server_boot_called = true;
    if (hb_sec)
        *hb_sec = 120U;
    return SYN_OCPP_REGISTRATION_ACCEPTED;
}

static SYN_OCPP_AuthorizationStatus on_server_auth(const char *id_tag, void *ctx)
{
    (void)id_tag;
    (void)ctx;
    return SYN_OCPP_AUTH_ACCEPTED;
}

static int32_t on_server_start_tx(uint32_t conn_id, const char *id_tag, uint32_t meter_start,
                                  void *ctx)
{
    (void)conn_id;
    (void)id_tag;
    (void)meter_start;
    (void)ctx;
    return 2026;
}

void test_ocpp_init_and_null_checks(void)
{
    char buf[128];
    size_t len = 0;
    SYN_OCPP_ChargePointInfo info = {"Vendor", "Model", "SN123", "v1.0"};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_init(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_set_callbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_server_init(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_server_set_callbacks(NULL, NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_boot_notification(NULL, &info, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_heartbeat(NULL, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_heartbeat(&g_ocpp_client, buf, 15, &len));
    syn_ocpp_tick(NULL, 10, buf, sizeof(buf), &len);
}

void test_ocpp_boot_notification_formatting(void)
{
    syn_ocpp_init(&g_ocpp_client);
    SYN_OCPP_ChargePointInfo info = {"SyntropicVendor", "EVSE-v2", "SN-9999", "2.0.1"};
    char buf[256];
    size_t len = 0;

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, buf, sizeof(buf), &len));
    TEST_ASSERT_TRUE(len > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "BootNotification"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "SyntropicVendor"));
}

void test_ocpp_heartbeat_and_status_formatting(void)
{
    syn_ocpp_init(&g_ocpp_client);
    char buf[256];
    size_t len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_heartbeat(&g_ocpp_client, buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Heartbeat"));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_heartbeat(&g_ocpp_client, buf, 10, &len));

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_format_status_notification(&g_ocpp_client, 1, SYN_OCPP_STATUS_CHARGING,
                                                    "NoError", buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "StatusNotification"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Charging"));
}

void test_ocpp_transaction_formatting(void)
{
    syn_ocpp_init(&g_ocpp_client);
    char buf[256];
    size_t len = 0;

    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_format_authorize(&g_ocpp_client, "RFID-TAG-123", buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Authorize"));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_start_transaction(&g_ocpp_client, 1, "RFID-TAG-123",
                                                                1000, buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "StartTransaction"));

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_stop_transaction(&g_ocpp_client, 42, 12500, "EVDisconnected",
                                                       buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "StopTransaction"));
}

void test_ocpp_meter_values_formatting(void)
{
    syn_ocpp_init(&g_ocpp_client);
    SYN_OCPP_MeterValues mv = {12500, 230, 160, 3680, 75};
    char buf[256];
    size_t len = 0;

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_meter_values(&g_ocpp_client, 1, &mv, buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "MeterValues"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "12500"));
}

void test_ocpp_process_call_result(void)
{
    syn_ocpp_init(&g_ocpp_client);
    g_reg_cb_called = false;
    g_auth_cb_called = false;
    syn_ocpp_set_callbacks(&g_ocpp_client, on_ocpp_reg, on_ocpp_auth, on_ocpp_start_tx,
                           on_ocpp_remote_start, on_ocpp_remote_stop, NULL);

    char tx_buf[256];
    size_t tx_len = 0U;
    SYN_OCPP_ChargePointInfo info = {"Vendor", "Model", "SN", "1.0"};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, tx_buf,
                                                                sizeof(tx_buf), &tx_len));

    const char *resp_json = "[3,\"1\",{\"status\":\"Accepted\",\"interval\":60}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, resp_json, strlen(resp_json),
                                                       NULL, 0, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_REGISTRATION_ACCEPTED, g_ocpp_client.registration_status);
    TEST_ASSERT_TRUE(g_reg_cb_called);
    TEST_ASSERT_FALSE(g_auth_cb_called);

    const char *resp_status_json = "[3,\"2\",{\"status\":\"Accepted\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, resp_status_json,
                                                       strlen(resp_status_json), NULL, 0, NULL));
}

void test_ocpp_process_remote_start_stop(void)
{
    syn_ocpp_init(&g_ocpp_client);
    g_remote_start_called = false;
    g_remote_stop_called = false;
    syn_ocpp_set_callbacks(&g_ocpp_client, on_ocpp_reg, on_ocpp_auth, on_ocpp_start_tx,
                           on_ocpp_remote_start, on_ocpp_remote_stop, NULL);

    const char *cmd_start =
        "[2,\"10\",\"RemoteStartTransaction\",{\"connectorId\":1,\"idTag\":\"RFID-101\"}]";
    char resp[128];
    size_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, cmd_start, strlen(cmd_start),
                                                       resp, sizeof(resp), &resp_len));
    TEST_ASSERT_TRUE(g_remote_start_called);
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));

    const char *cmd_stop = "[2,\"11\",\"RemoteStopTransaction\",{\"transactionId\":42}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, cmd_stop, strlen(cmd_stop),
                                                       resp, sizeof(resp), &resp_len));
    TEST_ASSERT_TRUE(g_remote_stop_called);

    syn_ocpp_init(&g_ocpp_client);
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, cmd_start, strlen(cmd_start),
                                                       resp, sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, cmd_stop, strlen(cmd_stop),
                                                       resp, sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));
}

void test_ocpp_tick(void)
{
    syn_ocpp_init(&g_ocpp_client);
    g_ocpp_client.heartbeat_timer_ms = 100;

    char hb_buf[128];
    size_t hb_len = 0;

    syn_ocpp_tick(&g_ocpp_client, 50, hb_buf, sizeof(hb_buf), &hb_len);
    TEST_ASSERT_EQUAL(0, hb_len);

    syn_ocpp_tick(&g_ocpp_client, 60, hb_buf, sizeof(hb_buf), &hb_len);
    TEST_ASSERT_TRUE(hb_len > 0);
    TEST_ASSERT_NOT_NULL(strstr(hb_buf, "Heartbeat"));
}

void test_ocpp_all_status_enums_and_small_buffers(void)
{
    syn_ocpp_init(&g_ocpp_client);
    char buf[256];
    size_t len = 0;

    SYN_OCPP_ChargePointStatus statuses[] = {
        SYN_OCPP_STATUS_AVAILABLE,     SYN_OCPP_STATUS_PREPARING,      SYN_OCPP_STATUS_CHARGING,
        SYN_OCPP_STATUS_SUSPENDED_EV,  SYN_OCPP_STATUS_SUSPENDED_EVSE, SYN_OCPP_STATUS_FINISHING,
        SYN_OCPP_STATUS_RESERVED,      SYN_OCPP_STATUS_UNAVAILABLE,    SYN_OCPP_STATUS_FAULTED,
        (SYN_OCPP_ChargePointStatus)99};

    for (size_t i = 0; i < sizeof(statuses) / sizeof(statuses[0]); i++) {
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_ocpp_format_status_notification(&g_ocpp_client, 1, statuses[i],
                                                              "NoError", buf, sizeof(buf), &len));
    }

    SYN_OCPP_ChargePointInfo info = {"Vendor", "Model", "SN", "v1"};
    SYN_OCPP_MeterValues mv = {100, 230, 16, 3600, 50};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_boot_notification(&g_ocpp_client, &info, buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_heartbeat(&g_ocpp_client, buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_status_notification(
                          &g_ocpp_client, 1, SYN_OCPP_STATUS_CHARGING, "NoError", buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_authorize(&g_ocpp_client, "TAG", buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_start_transaction(&g_ocpp_client, 1, "TAG",
                                                                           100, buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_stop_transaction(&g_ocpp_client, 1, 100,
                                                                          "Reason", buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_meter_values(&g_ocpp_client, 1, &mv, buf, 10, &len));

    /* NULL info fields default check */
    SYN_OCPP_ChargePointInfo null_info = {NULL, NULL, NULL, NULL};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &null_info, buf,
                                                                sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "SyntropicOS"));

    /* Buffer capacity truncation tests (SYN_ERROR) */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_format_boot_notification(&g_ocpp_client, &info, buf, 70, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_heartbeat(&g_ocpp_client, buf, 20, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_status_notification(&g_ocpp_client, 1,
                                                                     SYN_OCPP_STATUS_CHARGING,
                                                                     "NoError", buf, 70, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_authorize(
                                     &g_ocpp_client, "TAG-VERY-LONG-STRING-12345", buf, 50, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_start_transaction(&g_ocpp_client, 1,
                                                                   "TAG-VERY-LONG-STRING-12345",
                                                                   100, buf, 100, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_stop_transaction(&g_ocpp_client, 1, 100, "Reason",
                                                                  buf, 100, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_format_meter_values(&g_ocpp_client, 1, &mv, buf, 130, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_server_format_remote_start(&g_ocpp_server, 1,
                                                                     "TAG-VERY-LONG-STRING-12345",
                                                                     buf, 65, &len));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_server_format_remote_stop(&g_ocpp_server, 100, buf, 50, &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_server_format_remote_start(
                                             &g_ocpp_server, 1, NULL, buf, sizeof(buf), &len));
}

void test_ocpp_process_message_extended(void)
{
    syn_ocpp_init(&g_ocpp_client);
    g_reg_cb_called = false;
    g_auth_cb_called = false;
    g_start_tx_cb_called = false;
    syn_ocpp_set_callbacks(&g_ocpp_client, on_ocpp_reg, on_ocpp_auth, on_ocpp_start_tx, NULL, NULL,
                           NULL);

    char tx_buf[256];
    size_t tx_len = 0U;
    g_ocpp_client.message_counter = 99U;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_start_transaction(&g_ocpp_client, 1U, "RFID-100", 500U,
                                                        tx_buf, sizeof(tx_buf), &tx_len));

    const char *resp_tx =
        "[3,\"100\",{\"idTagInfo\":{\"status\":\"Accepted\"},\"transactionId\":1001}]";
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, resp_tx, strlen(resp_tx), NULL, 0, NULL));
    TEST_ASSERT_EQUAL(1001, g_ocpp_client.active_transaction_id);
    TEST_ASSERT_TRUE(g_start_tx_cb_called);
    TEST_ASSERT_FALSE(g_reg_cb_called);
    TEST_ASSERT_FALSE(g_auth_cb_called);

    const char *cmd_unknown = "[2,\"200\",\"DataTransfer\",{\"vendorId\":\"Vendor\"}]";
    char resp[128];
    size_t resp_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, cmd_unknown, strlen(cmd_unknown),
                                               resp, sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "[3,\"200\",{}]"));

    const char *err_msg = "[4,\"300\",\"NotSupported\",\"Action not supported\",{}]";
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, err_msg, strlen(err_msg), NULL, 0, NULL));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_process_message(&g_ocpp_client, "123", 3, resp,
                                                                  sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_process_message(&g_ocpp_client, "INVALID_FRAME", 13, resp,
                                                          sizeof(resp), &resp_len));
}

void test_ocpp_server_role(void)
{
    syn_ocpp_server_init(&g_ocpp_server);
    g_server_boot_called = false;
    syn_ocpp_server_set_callbacks(&g_ocpp_server, on_server_boot, on_server_auth,
                                  on_server_start_tx, NULL);

    char resp[256];
    size_t resp_len = 0;

    /* 1. Server processes station BootNotification */
    const char *req_boot = "[2,\"1\",\"BootNotification\",{\"chargePointVendor\":\"Vendor\"}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_boot, strlen(req_boot),
                                                      resp, sizeof(resp), &resp_len));
    TEST_ASSERT_TRUE(g_server_boot_called);
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));
    TEST_ASSERT_NOT_NULL(strstr(resp, "120"));

    /* 2. Server processes station Authorize */
    const char *req_auth = "[2,\"2\",\"Authorize\",{\"idTag\":\"RFID-1\"}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_auth, strlen(req_auth),
                                                      resp, sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));

    /* 3. Server processes station StartTransaction */
    const char *req_tx = "[2,\"3\",\"StartTransaction\",{\"connectorId\":1,\"idTag\":\"RFID-1\"}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_tx, strlen(req_tx), resp,
                                                      sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "2026"));

    /* 4. Server processes Heartbeat call */
    const char *req_hb = "[2,\"4\",\"Heartbeat\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_hb, strlen(req_hb), resp,
                                                      sizeof(resp), &resp_len));

    /* 5. Server formats RemoteStart / RemoteStop commands */
    char cmd_buf[128];
    size_t cmd_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_format_remote_start(&g_ocpp_server, 1, "REMOTE-TAG", cmd_buf,
                                                          sizeof(cmd_buf), &cmd_len));
    TEST_ASSERT_NOT_NULL(strstr(cmd_buf, "RemoteStartTransaction"));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_format_remote_stop(&g_ocpp_server, 2026, cmd_buf,
                                                                 sizeof(cmd_buf), &cmd_len));
    TEST_ASSERT_NOT_NULL(strstr(cmd_buf, "RemoteStopTransaction"));

    /* Null & error checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_server_format_remote_start(
                                             NULL, 1, "TAG", cmd_buf, sizeof(cmd_buf), &cmd_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_server_format_remote_stop(
                                             NULL, 1, cmd_buf, sizeof(cmd_buf), &cmd_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_server_process_message(NULL, req_boot, strlen(req_boot), resp,
                                                      sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_server_process_message(&g_ocpp_server, "INVALID_FRAME", 13, resp,
                                                      sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, "[3,\"1\",{}]", 10,
                                                              resp, sizeof(resp), &resp_len));

    /* Server process message without callbacks */
    syn_ocpp_server_init(&g_ocpp_server);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_boot, strlen(req_boot),
                                                      resp, sizeof(resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp, "Accepted"));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_auth, strlen(req_auth),
                                                      resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, req_tx, strlen(req_tx), resp,
                                                      sizeof(resp), &resp_len));
}

void test_ocpp_call_error_formatting(void)
{
    char buf[128];
    size_t len = 0;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_call_error(NULL, SYN_OCPP_ERROR_NOT_IMPLEMENTED, "Desc", buf,
                                                 sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_call_error("123", NULL, "Desc", buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_call_error("123", SYN_OCPP_ERROR_NOT_IMPLEMENTED, "Desc",
                                                 NULL, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_ocpp_format_call_error("123", SYN_OCPP_ERROR_NOT_IMPLEMENTED, "Desc", buf, 10, &len));

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_call_error("1001", SYN_OCPP_ERROR_NOT_IMPLEMENTED,
                                                         "Action unknown", buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "[4,\"1001\",\"NotImplemented\",\"Action unknown\",{}]"));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_call_error(
                                     "1001", SYN_OCPP_ERROR_NOT_IMPLEMENTED,
                                     "Very long error description string that exceeds capacity",
                                     buf, 40, &len));
}

void test_ocpp_21_features(void)
{
    char buf[256];
    size_t len = 0;

    /* Verify subprotocol strings */
    TEST_ASSERT_EQUAL_STRING("ocpp2.1", SYN_OCPP_SUBPROTOCOL_2_1);
    TEST_ASSERT_EQUAL_STRING("ocpp2.0.1", SYN_OCPP_SUBPROTOCOL_2_0_1);
    TEST_ASSERT_EQUAL_STRING("ocpp1.6", SYN_OCPP_SUBPROTOCOL_1_6);

    /* Verify DisplayMessage formatting */
    SYN_OCPP_DisplayMessage msg = {101, "Tariff Notice", "Off-peak rate active", 30};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_display_message(NULL, &msg, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_ocpp_format_display_message(&g_ocpp_client, NULL, buf,
                                                                         sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_display_message(&g_ocpp_client, &msg, buf, 10, &len));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_format_display_message(&g_ocpp_client, &msg, buf, sizeof(buf), &len));
    TEST_ASSERT_NOT_NULL(strstr(buf, "DisplayMessage"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Off-peak rate active"));
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_format_display_message(&g_ocpp_client, &msg, buf, 35, &len));

    /* Verify V2G Energy Transfer formatting */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_v2g_energy_transfer(NULL, SYN_OCPP_V2G_SCHEDULED, 11000, buf,
                                                          sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_ocpp_format_v2g_energy_transfer(&g_ocpp_client, SYN_OCPP_V2G_SCHEDULED,
                                                          11000, buf, 10, &len));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_v2g_energy_transfer(&g_ocpp_client, SYN_OCPP_V2G_SCHEDULED,
                                                          11000, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_OCPP_V2G_SCHEDULED, g_ocpp_client.v2g_mode);
    TEST_ASSERT_NOT_NULL(strstr(buf, "V2GEnergyTransfer"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Scheduled"));

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_v2g_energy_transfer(&g_ocpp_client, SYN_OCPP_V2G_DYNAMIC,
                                                          -5000, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL(SYN_OCPP_V2G_DYNAMIC, g_ocpp_client.v2g_mode);
    TEST_ASSERT_NOT_NULL(strstr(buf, "Dynamic"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "-5000"));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_v2g_energy_transfer(
                                     &g_ocpp_client, SYN_OCPP_V2G_DYNAMIC, -5000, buf, 35, &len));
}

static uint32_t g_r5_conn_id = 0U;
static char g_r5_tag[32];
static int32_t g_r5_tx_id = 0;
static uint32_t g_r5_meter_start = 0U;
static char g_r5_boot_vendor[32];
static char g_r5_boot_model[32];
static SYN_OCPP_AuthorizationStatus g_r5_auth_status = SYN_OCPP_AUTH_INVALID;
static SYN_OCPP_RegistrationStatus g_r5_reg_status = SYN_OCPP_REGISTRATION_ACCEPTED;

static bool on_r5_remote_start(uint32_t conn_id, const char *id_tag, void *ctx)
{
    (void)ctx;
    g_r5_conn_id = conn_id;
    (void)snprintf(g_r5_tag, sizeof(g_r5_tag), "%s", id_tag);
    return true;
}

static bool on_r5_remote_stop(int32_t tx_id, void *ctx)
{
    (void)ctx;
    g_r5_tx_id = tx_id;
    return (tx_id == 777);
}

static void on_r5_reg_cb(SYN_OCPP_RegistrationStatus status, uint32_t interval, void *ctx)
{
    (void)interval;
    (void)ctx;
    g_r5_reg_status = status;
    g_reg_cb_called = true;
}

static void on_r5_auth_cb(const char *id_tag, SYN_OCPP_AuthorizationStatus status, void *ctx)
{
    (void)ctx;
    (void)snprintf(g_r5_tag, sizeof(g_r5_tag), "%s", id_tag);
    g_r5_auth_status = status;
    g_auth_cb_called = true;
}

static SYN_OCPP_RegistrationStatus on_r5_srv_boot(const SYN_OCPP_ChargePointInfo *info,
                                                  uint32_t *hb_sec, void *ctx)
{
    (void)hb_sec;
    int mode = ctx ? *(const int *)ctx : 0;
    (void)snprintf(g_r5_boot_vendor, sizeof(g_r5_boot_vendor), "%s", info->charge_point_vendor);
    (void)snprintf(g_r5_boot_model, sizeof(g_r5_boot_model), "%s", info->charge_point_model);
    if (mode == 1) {
        return SYN_OCPP_REGISTRATION_PENDING;
    }
    return SYN_OCPP_REGISTRATION_REJECTED;
}

static SYN_OCPP_AuthorizationStatus on_r5_srv_auth(const char *id_tag, void *ctx)
{
    (void)ctx;
    (void)snprintf(g_r5_tag, sizeof(g_r5_tag), "%s", id_tag);
    if (strcmp(id_tag, "BLOCKED") == 0) {
        return SYN_OCPP_AUTH_BLOCKED;
    }
    if (strcmp(id_tag, "EXPIRED") == 0) {
        return SYN_OCPP_AUTH_EXPIRED;
    }
    if (strcmp(id_tag, "OK") == 0) {
        return SYN_OCPP_AUTH_ACCEPTED;
    }
    return SYN_OCPP_AUTH_INVALID;
}

static int32_t on_r5_srv_start_tx(uint32_t conn_id, const char *id_tag, uint32_t meter_start,
                                  void *ctx)
{
    (void)ctx;
    g_r5_conn_id = conn_id;
    (void)snprintf(g_r5_tag, sizeof(g_r5_tag), "%s", id_tag);
    g_r5_meter_start = meter_start;
    return 0; /* Exercise default server->next_transaction_id fallback */
}

void test_r5_a16_bounded_parse_pending_dispatch_and_deciamp(void)
{
    char tx_buf[256];
    char resp_buf[256];
    size_t tx_len = 0U;
    size_t resp_len = 0U;

    syn_ocpp_init(&g_ocpp_client);
    syn_ocpp_set_callbacks(&g_ocpp_client, on_r5_reg_cb, on_r5_auth_cb, on_ocpp_start_tx,
                           on_r5_remote_start, on_r5_remote_stop, NULL);

    /* 1. Heartbeat exact truncation boundary (22 bytes) without LCOV_EXCL */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_ocpp_format_heartbeat(&g_ocpp_client, tx_buf, 22U, &tx_len));

    /* 2. MeterValues formats current_a = 165 (16.5A) as "16.5" with unit "A" */
    SYN_OCPP_MeterValues mv = {5000U, 230U, 165U, 3795U, 80U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_meter_values(&g_ocpp_client, 2U, &mv, tx_buf,
                                                           sizeof(tx_buf), &tx_len));
    TEST_ASSERT_NOT_NULL(strstr(tx_buf, "{\"value\":\"16.5\",\"unit\":\"A\"}"));

    /* 3. Bounded scan: trailing poison past in_len must NEVER be read or matched */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_authorize(&g_ocpp_client, "TAG-AUTH-99", tx_buf,
                                                        sizeof(tx_buf), &tx_len));
    char non_nul_buf[128];
    const char *valid_prefix = "[3,\"3\",{\"idTagInfo\":{\"status\":\"Blocked\"}}]";
    size_t prefix_len = strlen(valid_prefix);
    memcpy(non_nul_buf, valid_prefix, prefix_len);
    /* Append bait after prefix_len without NUL terminator */
    const char *bait = "\"Accepted\",\"transactionId\":9999";
    memcpy(non_nul_buf + prefix_len, bait, strlen(bait));

    g_reg_cb_called = false;
    g_auth_cb_called = false;
    g_start_tx_cb_called = false;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, non_nul_buf, prefix_len, NULL, 0U, NULL));
    TEST_ASSERT_TRUE(g_auth_cb_called);
    TEST_ASSERT_FALSE(g_reg_cb_called);
    TEST_ASSERT_FALSE(g_start_tx_cb_called);
    TEST_ASSERT_EQUAL_STRING("TAG-AUTH-99", g_r5_tag);
    TEST_ASSERT_EQUAL(SYN_OCPP_AUTH_BLOCKED, g_r5_auth_status);

    /* Test Expired & Invalid & Pending/Rejected statuses + CALLERROR clearing pending */
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_authorize(&g_ocpp_client, "TAG-EXP", tx_buf,
                                                        sizeof(tx_buf), &tx_len));
    const char *exp_resp = "[3,\"4\",{\"idTagInfo\":{\"status\":\"Expired\"}}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, exp_resp, strlen(exp_resp),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_AUTH_EXPIRED, g_r5_auth_status);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_authorize(&g_ocpp_client, "TAG-INV", tx_buf,
                                                        sizeof(tx_buf), &tx_len));
    const char *inv_resp = "[3,\"5\",{\"idTagInfo\":{\"status\":123}}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, inv_resp, strlen(inv_resp),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_AUTH_INVALID, g_r5_auth_status);

    SYN_OCPP_ChargePointInfo info = {"RealVendor", "RealModel", "SN-42", "2.1"};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, tx_buf,
                                                                sizeof(tx_buf), &tx_len));
    const char *pend_resp = "[3,\"6\",{\"status\": \"Pending\",\"interval\": -5}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, pend_resp, strlen(pend_resp),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_REGISTRATION_PENDING, g_r5_reg_status);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, tx_buf,
                                                                sizeof(tx_buf), &tx_len));
    const char *rej_resp = "[3,\"7\",{\"status\":\"Rejected\",\"interval\":\"bad\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, rej_resp, strlen(rej_resp),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_REGISTRATION_REJECTED, g_r5_reg_status);

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_heartbeat(&g_ocpp_client, tx_buf, sizeof(tx_buf), &tx_len));
    const char *hb_resp = "[3,\"8\",{}]";
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, hb_resp, strlen(hb_resp), NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_ACTION_NONE, g_ocpp_client.pending_action);

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_heartbeat(&g_ocpp_client, tx_buf, sizeof(tx_buf), &tx_len));
    const char *call_err = "[4,\"9\",\"InternalError\",\"err\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, call_err, strlen(call_err),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(SYN_OCPP_ACTION_NONE, g_ocpp_client.pending_action);

    /* 4. Client RemoteStart / RemoteStop real JSON field parsing + malformed rejections */
    const char *rem_start =
        "[2,\"50\",\"RemoteStartTransaction\",{\"connectorId\": 3,\"idTag\": \"USER-BADGE-7\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, rem_start, strlen(rem_start),
                                                       resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_UINT32(3U, g_r5_conn_id);
    TEST_ASSERT_EQUAL_STRING("USER-BADGE-7", g_r5_tag);
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Accepted"));

    const char *rem_start_bad = "[2,\"51\",\"RemoteStartTransaction\",{\"connectorId\":1}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, rem_start_bad, strlen(rem_start_bad),
                                               resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Rejected"));

    const char *rem_stop_ok = "[2,\"52\",\"RemoteStopTransaction\",{\"transactionId\": 777}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, rem_stop_ok, strlen(rem_stop_ok),
                                               resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_INT32(777, g_r5_tx_id);
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Accepted"));

    const char *rem_stop_rej = "[2,\"53\",\"RemoteStopTransaction\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, rem_stop_rej, strlen(rem_stop_rej),
                                               resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Rejected"));

    /* Malformed msg_id quotes / overlong string value exercises parser guards */
    const char *no_quote_call = "[2,99,{}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, no_quote_call, strlen(no_quote_call),
                                               resp_buf, sizeof(resp_buf), &resp_len));
    const char *unclosed_id = "[2,\"99";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, unclosed_id, strlen(unclosed_id),
                                               resp_buf, sizeof(resp_buf), &resp_len));

    /* 5. CSMS Server parses real BootNotification, Authorize, and StartTransaction fields */
    int srv_mode = 1;
    syn_ocpp_server_init(&g_ocpp_server);
    syn_ocpp_server_set_callbacks(&g_ocpp_server, on_r5_srv_boot, on_r5_srv_auth,
                                  on_r5_srv_start_tx, &srv_mode);

    const char *srv_boot = "[2,\"1\",\"BootNotification\",{\"chargePointVendor\":\"ABB\","
                           "\"chargePointModel\":\"TerraAC\"}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, srv_boot, strlen(srv_boot),
                                                      resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_STRING("ABB", g_r5_boot_vendor);
    TEST_ASSERT_EQUAL_STRING("TerraAC", g_r5_boot_model);
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Pending"));

    srv_mode = 2;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_server_process_message(&g_ocpp_server, srv_boot, strlen(srv_boot),
                                                      resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Rejected"));

    const char *srv_auth_blk = "[2,\"2\",\"Authorize\",{\"idTag\":\"BLOCKED\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_auth_blk,
                                                              strlen(srv_auth_blk), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Blocked"));

    const char *srv_auth_exp = "[2,\"3\",\"Authorize\",{\"idTag\":\"EXPIRED\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_auth_exp,
                                                              strlen(srv_auth_exp), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Expired"));

    const char *srv_auth_bad = "[2,\"4\",\"Authorize\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_auth_bad,
                                                              strlen(srv_auth_bad), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Invalid"));

    const char *srv_tx_req = "[2,\"5\",\"StartTransaction\",{\"connectorId\":2,\"idTag\":"
                             "\"SUPERLONGIDENTIFIERSTRING0123456789\",\"meterStart\":4321}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_tx_req,
                                                              strlen(srv_tx_req), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_UINT32(2U, g_r5_conn_id);
    TEST_ASSERT_EQUAL_UINT32(4321U, g_r5_meter_start);
    TEST_ASSERT_EQUAL(31U, strlen(g_r5_tag));

    const char *srv_tx_bad = "[2,\"6\",\"StartTransaction\",{\"connectorId\":-1}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_tx_bad,
                                                              strlen(srv_tx_bad), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "Invalid"));

    const char *srv_unterm_str = "[2,7,\"Authorize\",{\"idTag\":\"UNTERM}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_unterm_str,
                                                              strlen(srv_unterm_str), resp_buf,
                                                              sizeof(resp_buf), &resp_len));

    /* Whitespace before ':' in JSON string/int fields, unknown client msg_type, and unquoted
     * server msg_id */
    const char *ws_start =
        "[2,\"60\",\"RemoteStartTransaction\",{\"connectorId\" : 5, \"idTag\" : \"WS-TAG\"}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, ws_start, strlen(ws_start),
                                                       resp_buf, sizeof(resp_buf), &resp_len));
    TEST_ASSERT_EQUAL_UINT32(5U, g_r5_conn_id);
    TEST_ASSERT_EQUAL_STRING("WS-TAG", g_r5_tag);

    const char *unk_type = "[9,\"61\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, unk_type, strlen(unk_type),
                                                       resp_buf, sizeof(resp_buf), &resp_len));

    const char *srv_empty_id_hb = "[2,\"\",\"Heartbeat\",{}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_server_process_message(&g_ocpp_server, srv_empty_id_hb,
                                                              strlen(srv_empty_id_hb), resp_buf,
                                                              sizeof(resp_buf), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(resp_buf, "[3,\"1\""));

    /* 6. Concurrent BootNotification + StartTransaction + periodic Heartbeat must not clobber
     * pending confirmations */
    syn_ocpp_init(&g_ocpp_client);
    syn_ocpp_set_callbacks(&g_ocpp_client, on_r5_reg_cb, on_r5_auth_cb, on_ocpp_start_tx,
                           on_r5_remote_start, on_r5_remote_stop, NULL);
    g_reg_cb_called = false;
    g_start_tx_cb_called = false;

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, tx_buf,
                                                                sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_start_transaction(&g_ocpp_client, 1U, "TAG-CONC", 100U,
                                                        tx_buf, sizeof(tx_buf), &tx_len));
    g_ocpp_client.heartbeat_timer_ms = 10U;
    syn_ocpp_tick(&g_ocpp_client, 20U, tx_buf, sizeof(tx_buf), &tx_len);
    TEST_ASSERT_NOT_NULL(strstr(tx_buf, "Heartbeat"));

    /* Malformed unquoted CallResult msg_id ignored safely */
    const char *bad_res_id = "[3,99,{}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, bad_res_id,
                                                       strlen(bad_res_id), NULL, 0U, NULL));

    /* Heartbeat.conf ("3") arrives first while "1" and "2" remain pending */
    const char *conc_hb = "[3,\"3\",{}]";
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, conc_hb, strlen(conc_hb), NULL, 0U, NULL));
    TEST_ASSERT_NOT_EQUAL(SYN_OCPP_ACTION_NONE, g_ocpp_client.pending_action);

    /* BootNotification.conf ("1") and StartTransaction.conf ("2") still dispatch cleanly */
    const char *conc_boot = "[3,\"1\",{\"status\":\"Accepted\",\"interval\":45}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, conc_boot, strlen(conc_boot),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_TRUE(g_reg_cb_called);
    TEST_ASSERT_EQUAL(SYN_OCPP_REGISTRATION_ACCEPTED, g_ocpp_client.registration_status);
    TEST_ASSERT_EQUAL_UINT32(45U, g_ocpp_client.heartbeat_interval_sec);

    const char *conc_tx =
        "[3,\"2\",{\"idTagInfo\":{\"status\":\"Accepted\"},\"transactionId\":4242}]";
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_ocpp_process_message(&g_ocpp_client, conc_tx, strlen(conc_tx), NULL, 0U, NULL));
    TEST_ASSERT_TRUE(g_start_tx_cb_called);
    TEST_ASSERT_EQUAL_INT32(4242, g_ocpp_client.active_transaction_id);
    TEST_ASSERT_EQUAL(SYN_OCPP_ACTION_NONE, g_ocpp_client.pending_action);
}

static char s_pro18_buf[512];
static char s_pro18_resp[256];
static char s_pro18_long[140];

void test_pro18_ocpp_i32_sat_slots_and_json_escape(void)
{
    size_t tx_len = 0U;
    size_t resp_len = 0U;

    syn_ocpp_init(&g_ocpp_client);
    syn_ocpp_set_callbacks(&g_ocpp_client, on_r5_reg_cb, on_r5_auth_cb, on_ocpp_start_tx,
                           on_r5_remote_start, on_r5_remote_stop, NULL);

    /* 1. i32 overflow/underflow rejection, INT32_MIN/MAX boundaries, and ms saturation */
    SYN_OCPP_ChargePointInfo info = {"Vendor", "Model", "SN", "1.0"};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, s_pro18_buf,
                                                                sizeof(s_pro18_buf), &tx_len));
    const char *boot_ovf = "[3,\"1\",{\"status\":\"Accepted\",\"interval\":2147483648}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, boot_ovf, strlen(boot_ovf),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_UINT32(60U, g_ocpp_client.heartbeat_interval_sec);

    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_boot_notification(&g_ocpp_client, &info, s_pro18_buf,
                                                                sizeof(s_pro18_buf), &tx_len));
    const char *boot_sat = "[3,\"2\",{\"status\":\"Accepted\",\"interval\":2147483647}]";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_process_message(&g_ocpp_client, boot_sat, strlen(boot_sat),
                                                       NULL, 0U, NULL));
    TEST_ASSERT_EQUAL_UINT32(2147483647U, g_ocpp_client.heartbeat_interval_sec);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, g_ocpp_client.heartbeat_timer_ms);

    g_ocpp_client.heartbeat_timer_ms = 5U;
    syn_ocpp_tick(&g_ocpp_client, 10U, s_pro18_buf, sizeof(s_pro18_buf), &tx_len);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, g_ocpp_client.heartbeat_timer_ms);

    g_ocpp_client.heartbeat_interval_sec = 0U;
    g_ocpp_client.heartbeat_timer_ms = 5U;
    syn_ocpp_tick(&g_ocpp_client, 10U, NULL, 0U, NULL);
    TEST_ASSERT_EQUAL_UINT32(60000U, g_ocpp_client.heartbeat_timer_ms);

    const char *stop_udf = "[2,\"20\",\"RemoteStopTransaction\",{\"transactionId\":-2147483649}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, stop_udf, strlen(stop_udf),
                                               s_pro18_resp, sizeof(s_pro18_resp), &resp_len));
    TEST_ASSERT_NOT_NULL(strstr(s_pro18_resp, "Rejected"));

    const char *stop_min = "[2,\"21\",\"RemoteStopTransaction\",{\"transactionId\":-2147483648}]";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_process_message(&g_ocpp_client, stop_min, strlen(stop_min),
                                               s_pro18_resp, sizeof(s_pro18_resp), &resp_len));
    TEST_ASSERT_EQUAL_INT32((-2147483647 - 1), g_r5_tx_id);

    /* 2. JSON string escaping (quotes, backslashes, \n, \r, \t, control < 0x20, NULLs, overflow) */
    const char *dirty = "A\"B\\C\nD\rE\tF\x01G";
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_authorize(&g_ocpp_client, dirty, s_pro18_buf,
                                                        sizeof(s_pro18_buf), &tx_len));
    TEST_ASSERT_NOT_NULL(strstr(s_pro18_buf, "\"A\\\"B\\\\C\\nD\\rE\\tF\\u0001G\""));

    SYN_OCPP_DisplayMessage dmsg = {7U, NULL, NULL, 10U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_display_message(&g_ocpp_client, &dmsg, s_pro18_buf,
                                                              sizeof(s_pro18_buf), &tx_len));
    TEST_ASSERT_EQUAL(SYN_OK, syn_ocpp_format_status_notification(
                                  &g_ocpp_client, 1U, SYN_OCPP_STATUS_AVAILABLE, NULL, s_pro18_buf,
                                  sizeof(s_pro18_buf), &tx_len));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_stop_transaction(&g_ocpp_client, 1, 100U, NULL, s_pro18_buf,
                                                       sizeof(s_pro18_buf), &tx_len));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_ocpp_format_call_error("99", SYN_OCPP_ERROR_GENERIC_ERROR, NULL,
                                                 s_pro18_buf, sizeof(s_pro18_buf), &tx_len));

    memset(s_pro18_long, 'X', sizeof(s_pro18_long) - 1U);
    s_pro18_long[sizeof(s_pro18_long) - 1U] = '\0';
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_ocpp_format_authorize(&g_ocpp_client, s_pro18_long, s_pro18_buf,
                                                sizeof(s_pro18_buf), &tx_len));
}

void run_ocpp_tests(void)
{
    RUN_TEST(test_ocpp_init_and_null_checks);
    RUN_TEST(test_ocpp_boot_notification_formatting);
    RUN_TEST(test_ocpp_heartbeat_and_status_formatting);
    RUN_TEST(test_ocpp_transaction_formatting);
    RUN_TEST(test_ocpp_meter_values_formatting);
    RUN_TEST(test_ocpp_process_call_result);
    RUN_TEST(test_ocpp_process_remote_start_stop);
    RUN_TEST(test_ocpp_tick);
    RUN_TEST(test_ocpp_all_status_enums_and_small_buffers);
    RUN_TEST(test_ocpp_process_message_extended);
    RUN_TEST(test_ocpp_server_role);
    RUN_TEST(test_ocpp_call_error_formatting);
    RUN_TEST(test_ocpp_21_features);
    RUN_TEST(test_r5_a16_bounded_parse_pending_dispatch_and_deciamp);
    RUN_TEST(test_pro18_ocpp_i32_sat_slots_and_json_escape);
}
