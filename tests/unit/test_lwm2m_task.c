/**
 * @file test_lwm2m_task.c
 * @brief Unit tests for Autonomous OMA LwM2M Client Supervisor Task.
 */

#include "syntropic/net/syn_coap.h"
#include "syntropic/net/syn_transport.h"
#include "syntropic/proto/syn_lwm2m.h"
#include "syntropic/proto/syn_lwm2m_task.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

#include <string.h>

/* ── Mock Transport ──────────────────────────────────────────────────────── */

#define MOCK_TX_RING_SZ 2048U
#define MOCK_RX_RING_SZ 2048U

static uint8_t g_mock_tx_buf[MOCK_TX_RING_SZ];
static size_t g_mock_tx_len = 0U;

static uint8_t g_mock_rx_buf[MOCK_RX_RING_SZ];
static size_t g_mock_rx_len = 0U;
static size_t g_mock_rx_read_pos = 0U;

static bool mock_transport_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (len > sizeof(g_mock_tx_buf)) {
        return false;
    }
    (void)memcpy(g_mock_tx_buf, data, len);
    g_mock_tx_len = len;
    return true;
}

static bool mock_transport_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    (void)ctx;
    if (g_mock_rx_len == 0U || g_mock_rx_read_pos >= g_mock_rx_len) {
        if (out_len != NULL) {
            *out_len = 0U;
        }
        return false;
    }

    size_t avail = g_mock_rx_len - g_mock_rx_read_pos;
    size_t copy_len = (avail < max_len) ? avail : max_len;
    (void)memcpy(data, &g_mock_rx_buf[g_mock_rx_read_pos], copy_len);
    g_mock_rx_read_pos += copy_len;
    if (out_len != NULL) {
        *out_len = copy_len;
    }
    return true;
}

static void mock_transport_feed_rx(const void *data, size_t len)
{
    if (len <= sizeof(g_mock_rx_buf)) {
        (void)memcpy(g_mock_rx_buf, data, len);
        g_mock_rx_len = len;
        g_mock_rx_read_pos = 0U;
    }
}

static void mock_transport_reset(void)
{
    g_mock_tx_len = 0U;
    g_mock_rx_len = 0U;
    g_mock_rx_read_pos = 0U;
    (void)memset(g_mock_tx_buf, 0, sizeof(g_mock_tx_buf));
    (void)memset(g_mock_rx_buf, 0, sizeof(g_mock_rx_buf));
}

static SYN_Transport g_transport = {
    .send = mock_transport_send,
    .recv = mock_transport_recv,
    .has_data = NULL,
    .ctx = NULL,
};

/* ── Callback Recording State ────────────────────────────────────────────── */

static uint32_t g_reboot_called = 0U;
static uint32_t g_reset_called = 0U;
static uint32_t g_fw_update_called = 0U;
static char g_fw_update_uri[64];
static uint32_t g_state_changes = 0U;
static SYN_LwM2M_ClientState g_last_old_state;
static SYN_LwM2M_ClientState g_last_new_state;

static void on_reboot_cb(void *user_data)
{
    (void)user_data;
    g_reboot_called++;
}

static void on_reset_cb(void *user_data)
{
    (void)user_data;
    g_reset_called++;
}

static void on_fw_update_cb(const char *uri, void *user_data)
{
    (void)user_data;
    g_fw_update_called++;
    if (uri != NULL) {
        (void)strncpy(g_fw_update_uri, uri, sizeof(g_fw_update_uri) - 1U);
        g_fw_update_uri[sizeof(g_fw_update_uri) - 1U] = '\0';
    }
}

static void on_state_cb(SYN_LwM2M_ClientState old_st, SYN_LwM2M_ClientState new_st, void *user_data)
{
    (void)user_data;
    g_state_changes++;
    g_last_old_state = old_st;
    g_last_new_state = new_st;
}

/* ── Test Fixture Setup / Teardown ───────────────────────────────────────── */

static SYN_LwM2M_Client g_client;
static SYN_LwM2M_Task g_task;
static SYN_LwM2M_DeviceContext g_dev_ctx;
static SYN_LwM2M_FirmwareContext g_fw_ctx;
static SYN_LwM2M_SensorContext g_temp_ctx;
static SYN_LwM2M_Object g_dev_obj;
static SYN_LwM2M_Object g_fw_obj;
static SYN_LwM2M_Object g_temp_obj;

static uint8_t g_rx_scratch[512];
static uint8_t g_tx_scratch[512];

static void lwm2m_task_test_setup(void)
{
    mock_transport_reset();

    g_reboot_called = 0U;
    g_reset_called = 0U;
    g_fw_update_called = 0U;
    g_fw_update_uri[0] = '\0';
    g_state_changes = 0U;

    (void)memset(&g_dev_ctx, 0, sizeof(g_dev_ctx));
    g_dev_ctx.manufacturer = "SyntropicOS";
    g_dev_ctx.model_number = "Edge-v1";
    g_dev_ctx.serial_number = "SN-9999";
    g_dev_ctx.firmware_ver = "1.0.0";

    (void)memset(&g_fw_ctx, 0, sizeof(g_fw_ctx));
    (void)memset(&g_temp_ctx, 0, sizeof(g_temp_ctx));
    g_temp_ctx.sensor_value = 23.5;
    g_temp_ctx.unit = "Cel";

    (void)syn_lwm2m_client_init(&g_client, "syn-edge-01", 300U, &g_transport);

    g_dev_obj = syn_lwm2m_make_device_object(&g_dev_ctx);
    g_fw_obj = syn_lwm2m_make_firmware_object(&g_fw_ctx);
    g_temp_obj = syn_lwm2m_make_temperature_object(&g_temp_ctx);

    (void)syn_lwm2m_register_object(&g_client, &g_dev_obj);
    (void)syn_lwm2m_register_object(&g_client, &g_fw_obj);
    (void)syn_lwm2m_register_object(&g_client, &g_temp_obj);

    SYN_LwM2M_TaskConfig cfg = {
        .client = &g_client,
        .transport = &g_transport,
        .coaps_client = NULL,
        .retry_backoff_ms = 2000U,
        .on_reboot = on_reboot_cb,
        .on_reset = on_reset_cb,
        .on_fw_update = on_fw_update_cb,
        .on_state = on_state_cb,
        .user_data = (void *)0x1234,
        .rx_buf = g_rx_scratch,
        .rx_buf_size = sizeof(g_rx_scratch),
        .tx_buf = g_tx_scratch,
        .tx_buf_size = sizeof(g_tx_scratch),
    };

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_init(&g_task, &cfg));
}

/* ── Unit Test Cases ─────────────────────────────────────────────────────── */

void test_lwm2m_task_init_null_and_validation(void)
{
    SYN_LwM2M_Task task;
    SYN_LwM2M_TaskConfig cfg;
    (void)memset(&cfg, 0, sizeof(cfg));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(NULL, &cfg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, NULL));

    /* Missing client / transport */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));
    cfg.client = &g_client;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));
    cfg.transport = &g_transport;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));

    /* Missing or too small scratch buffers */
    cfg.rx_buf = g_rx_scratch;
    cfg.rx_buf_size = 32U; /* Too small (<64) */
    cfg.tx_buf = g_tx_scratch;
    cfg.tx_buf_size = 512U;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));

    cfg.rx_buf_size = 512U;
    cfg.tx_buf = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));

    cfg.tx_buf = g_tx_scratch;
    cfg.tx_buf_size = 32U; /* Too small (<64) */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_init(&task, &cfg));

    cfg.tx_buf_size = 512U;
    cfg.retry_backoff_ms = 0U; /* Should default to 5000 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_init(&task, &cfg));
    TEST_ASSERT_EQUAL_UINT32(5000U, task.cfg.retry_backoff_ms);

    /* Test trigger functions with NULL */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_trigger_update(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_deregister(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_notify_changed(NULL, 3303, 0, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_step(NULL, 1000U));
}

void test_lwm2m_task_registration_success_flow(void)
{
    lwm2m_task_test_setup();

    /* Initial state: DEREGISTERED */
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Step 1: Task initiates registration request */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, g_state_changes);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_last_old_state);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_last_new_state);

    /* Inspect outgoing CoAP message */
    SYN_CoapMsg tx_coap;
    SYN_CoapOption tx_opts[8];
    size_t tx_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_coap_parse(&tx_coap, tx_opts, 8U, &tx_opt_cnt, g_mock_tx_buf, g_mock_tx_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_TYPE_CON, tx_coap.type);
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_POST, tx_coap.code);

    /* Simulate Server 2.01 Created response with Location-Path: rd/demo9876 */
    SYN_CoapMsg resp;
    (void)memset(&resp, 0, sizeof(resp));
    resp.type = COAP_TYPE_ACK;
    resp.code = COAP_RESP_CREATED;
    resp.msg_id = tx_coap.msg_id;
    resp.token_len = tx_coap.token_len;
    (void)memcpy(resp.token, tx_coap.token, tx_coap.token_len);

    SYN_CoapOption resp_opts[2];
    resp_opts[0].num = COAP_OPT_LOCATION_PATH;
    resp_opts[0].val = (const uint8_t *)"rd";
    resp_opts[0].len = 2U;
    resp_opts[1].num = COAP_OPT_LOCATION_PATH;
    resp_opts[1].val = (const uint8_t *)"demo9876";
    resp_opts[1].len = 8U;

    uint8_t server_resp_frame[128];
    size_t srv_len =
        syn_coap_serialize(&resp, resp_opts, 2U, server_resp_frame, sizeof(server_resp_frame));
    TEST_ASSERT_TRUE(srv_len > 0U);

    /* Feed response into mock transport */
    mock_transport_reset();
    mock_transport_feed_rx(server_resp_frame, srv_len);

    /* Step 2: Task processes 2.01 Created response */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_STRING("rd/demo9876", g_client.location_path);
    TEST_ASSERT_EQUAL_UINT32(2U, g_state_changes);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_last_old_state);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_last_new_state);
}

void test_lwm2m_task_registration_failure_and_retry_backoff(void)
{
    lwm2m_task_test_setup();

    /* Step 1: Send registration request */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);

    /* Simulate Server 4.00 Bad Request error response */
    SYN_CoapMsg resp;
    (void)memset(&resp, 0, sizeof(resp));
    resp.type = COAP_TYPE_ACK;
    resp.code = COAP_RESP_BAD_REQ;
    resp.msg_id = g_task.pending_msg_id;
    uint8_t server_resp_frame[64];
    size_t srv_len =
        syn_coap_serialize(&resp, NULL, 0U, server_resp_frame, sizeof(server_resp_frame));
    mock_transport_feed_rx(server_resp_frame, srv_len);

    /* Step 2: Handle rejection */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(1U, g_task.retry_count);

    /* Step 3: Step before retry_backoff_ms (2000ms) has elapsed - should NOT transmit */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 2000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Step 4: Step after retry backoff has elapsed (1000 + 2000 = 3000ms) - should retry */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
}

void test_lwm2m_task_lifetime_update_and_force_trigger(void)
{
    test_lwm2m_task_registration_success_flow();

    /* Lifetime is 300s (300,000ms), 80% renewal is at 240,000ms */
    /* Step at 200,000ms: no update needed */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 200000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Step at 242,000ms: exceeds 80% threshold -> sends update request */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 242000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_UPDATING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Verify outgoing update is POST rd/demo9876 with zero leaked stack payload */
    SYN_CoapMsg update_coap;
    SYN_CoapOption update_opts[8];
    size_t update_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_coap_parse(&update_coap, update_opts, 8U, &update_opt_cnt,
                                                 g_mock_tx_buf, g_mock_tx_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_POST, update_coap.code);
    TEST_ASSERT_EQUAL_UINT32(0U, update_coap.payload_len);

    /* Simulate 2.04 Changed response from server */
    SYN_CoapMsg resp;
    (void)memset(&resp, 0, sizeof(resp));
    resp.type = COAP_TYPE_ACK;
    resp.code = COAP_RESP_CHANGED;
    resp.msg_id = update_coap.msg_id;
    uint8_t server_resp_frame[64];
    size_t srv_len =
        syn_coap_serialize(&resp, NULL, 0U, server_resp_frame, sizeof(server_resp_frame));
    mock_transport_reset();
    mock_transport_feed_rx(server_resp_frame, srv_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 242100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);

    /* Test Force Update Trigger */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_trigger_update(&g_task));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 242200U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_UPDATING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Test update rejection (4.04 Not Found) */
    (void)memset(&resp, 0, sizeof(resp));
    resp.type = COAP_TYPE_ACK;
    resp.code = COAP_RESP_NOT_FOUND;
    resp.msg_id = g_task.pending_msg_id;
    srv_len = syn_coap_serialize(&resp, NULL, 0U, server_resp_frame, sizeof(server_resp_frame));
    mock_transport_reset();
    mock_transport_feed_rx(server_resp_frame, srv_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 242300U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
}

void test_lwm2m_task_downlink_requests_and_execute_hooks(void)
{
    test_lwm2m_task_registration_success_flow();

    /* 1. Downlink GET /3/0/0 (Device Manufacturer) */
    SYN_CoapMsg get_req;
    (void)memset(&get_req, 0, sizeof(get_req));
    get_req.type = COAP_TYPE_CON;
    get_req.code = COAP_CODE_GET;
    get_req.msg_id = 0x1010;

    SYN_CoapOption get_opts[3];
    get_opts[0].num = COAP_OPT_URI_PATH;
    get_opts[0].val = (const uint8_t *)"3";
    get_opts[0].len = 1U;
    get_opts[1].num = COAP_OPT_URI_PATH;
    get_opts[1].val = (const uint8_t *)"0";
    get_opts[1].len = 1U;
    get_opts[2].num = COAP_OPT_URI_PATH;
    get_opts[2].val = (const uint8_t *)"0";
    get_opts[2].len = 1U;

    uint8_t req_frame[128];
    size_t req_len = syn_coap_serialize(&get_req, get_opts, 3U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    SYN_CoapMsg resp;
    SYN_CoapOption resp_opts[8];
    size_t resp_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_coap_parse(&resp, resp_opts, 8U, &resp_opt_cnt, g_mock_tx_buf, g_mock_tx_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CONTENT, resp.code);

    /* 2. Downlink POST /3/0/4 (Device Reboot Command) */
    SYN_CoapMsg reboot_req;
    (void)memset(&reboot_req, 0, sizeof(reboot_req));
    reboot_req.type = COAP_TYPE_CON;
    reboot_req.code = COAP_CODE_POST;
    reboot_req.msg_id = 0x1011;
    get_opts[2].val = (const uint8_t *)"4";
    req_len = syn_coap_serialize(&reboot_req, get_opts, 3U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);

    TEST_ASSERT_EQUAL_UINT32(0U, g_reboot_called);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3100U));
    TEST_ASSERT_EQUAL_UINT32(1U, g_reboot_called);

    /* 3. Downlink POST /3/0/5 (Device Factory Reset Command) */
    get_opts[2].val = (const uint8_t *)"5";
    req_len = syn_coap_serialize(&reboot_req, get_opts, 3U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);

    TEST_ASSERT_EQUAL_UINT32(0U, g_reset_called);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3200U));
    TEST_ASSERT_EQUAL_UINT32(1U, g_reset_called);

    /* 4. Downlink POST /5/0/2 (Firmware Update Command) */
    (void)strncpy(g_fw_ctx.package_uri, "coap://ota.syntropic.io/app_v2.bin",
                  sizeof(g_fw_ctx.package_uri) - 1U);
    get_opts[0].val = (const uint8_t *)"5";
    get_opts[1].val = (const uint8_t *)"0";
    get_opts[2].val = (const uint8_t *)"2";
    req_len = syn_coap_serialize(&reboot_req, get_opts, 3U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);

    TEST_ASSERT_EQUAL_UINT32(0U, g_fw_update_called);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3300U));
    TEST_ASSERT_EQUAL_UINT32(1U, g_fw_update_called);
    TEST_ASSERT_EQUAL_STRING("coap://ota.syntropic.io/app_v2.bin", g_fw_update_uri);
}

void test_lwm2m_task_observe_and_notify_scheduler(void)
{
    test_lwm2m_task_registration_success_flow();

    /* Send Observe registration: GET /3303/0/5700 with Observe=0 */
    SYN_CoapMsg obs_req;
    (void)memset(&obs_req, 0, sizeof(obs_req));
    obs_req.type = COAP_TYPE_CON;
    obs_req.code = COAP_CODE_GET;
    obs_req.msg_id = 0x2020;
    obs_req.token_len = 2U;
    obs_req.token[0] = 0xAA;
    obs_req.token[1] = 0x55;

    uint8_t obs_val = 0U;
    SYN_CoapOption obs_opts[4];
    obs_opts[0].num = COAP_OPT_OBSERVE;
    obs_opts[0].val = &obs_val;
    obs_opts[0].len = 0U;
    obs_opts[1].num = COAP_OPT_URI_PATH;
    obs_opts[1].val = (const uint8_t *)"3303";
    obs_opts[1].len = 4U;
    obs_opts[2].num = COAP_OPT_URI_PATH;
    obs_opts[2].val = (const uint8_t *)"0";
    obs_opts[2].len = 1U;
    obs_opts[3].num = COAP_OPT_URI_PATH;
    obs_opts[3].val = (const uint8_t *)"5700";
    obs_opts[3].len = 4U;

    uint8_t req_frame[128];
    size_t req_len = syn_coap_serialize(&obs_req, obs_opts, 4U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 5000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Verify observation slot is active and configure pmin=2s, pmax=10s */
    TEST_ASSERT_TRUE(g_client.observations[0].active);
    g_client.observations[0].pmin = 2U;
    g_client.observations[0].pmax = 10U;
    g_client.observations[0].last_notify_ms = 5000U;

    /* Notify changed within pmin window (1 second later at 6000ms): suppressed */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 6000U));
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Notify changed after pmin elapsed (3 seconds later at 8000ms): notification fires */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 8000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Verify notification packet */
    SYN_CoapMsg notif;
    SYN_CoapOption notif_opts[8];
    size_t notif_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_coap_parse(&notif, notif_opts, 8U, &notif_opt_cnt,
                                                 g_mock_tx_buf, g_mock_tx_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CONTENT, notif.code);

    /* Test periodic pmax expiration (10 seconds later at 19000ms) without manual change */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 19000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
}

void test_lwm2m_task_graceful_deregistration(void)
{
    test_lwm2m_task_registration_success_flow();

    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_deregister(&g_task));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 10000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_STRING("", g_client.location_path);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Verify outgoing DELETE rd/demo9876 message */
    SYN_CoapMsg del_coap;
    SYN_CoapOption del_opts[8];
    size_t del_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_coap_parse(&del_coap, del_opts, 8U, &del_opt_cnt,
                                                 g_mock_tx_buf, g_mock_tx_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_DELETE, del_coap.code);
}

void test_lwm2m_task_scheduler_protothread_integration(void)
{
    lwm2m_task_test_setup();

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task sched_task;
    (void)memset(&sched_task, 0, sizeof(sched_task));
    sched_task.user_data = &g_task;

    /* NULL protection */
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_lwm2m_task_pt(NULL, &sched_task));
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_lwm2m_task_pt(&pt, NULL));
    sched_task.user_data = NULL;
    TEST_ASSERT_EQUAL_INT(PT_EXITED, syn_lwm2m_task_pt(&pt, &sched_task));

    /* Execution pass step */
    sched_task.user_data = &g_task;
    SYN_PT_Status status = syn_lwm2m_task_pt(&pt, &sched_task);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, status);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);

    /* Test protothread loop exit on step failure */
    g_task.cfg.client = NULL;
    status = syn_lwm2m_task_pt(&pt, &sched_task);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, status);
}

void test_lwm2m_task_dtls_coaps_integration(void)
{
    lwm2m_task_test_setup();

    SYN_DTLS_Config dtls_cfg = {
        .mode = SYN_DTLS_AUTH_MODE_MTLS,
        .server_name = "coaps.syntropic.local",
        .peer_pubkey = NULL,
        .root_ca = NULL,
    };

    uint8_t dtls_rx[512];
    uint8_t dtls_tx[512];
    SYN_CoapsClient coaps_client;
    TEST_ASSERT_TRUE(syn_coaps_client_init(&coaps_client, &dtls_cfg, &g_transport, dtls_rx,
                                           sizeof(dtls_rx), dtls_tx, sizeof(dtls_tx)));

    g_task.cfg.coaps_client = &coaps_client;

    /* Handshake failure: step returns SYN_ERROR and protothread exits without spinning */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_lwm2m_task_step(&g_task, 1000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    SYN_PT hs_pt;
    PT_INIT(&hs_pt);
    SYN_Task hs_sched_task = {0};
    hs_sched_task.user_data = &g_task;
    TEST_ASSERT_EQUAL_INT(PT_ENDED, syn_lwm2m_task_pt(&hs_pt, &hs_sched_task));

    /* Handshake established: step 1 sends ClientHello, step 2 finishes DTLS & registers, step 3
     * reuses session */
    uint8_t psk[32] = {0x01};
    SYN_DTLS_Config psk_cfg = {
        .mode = SYN_DTLS_AUTH_MODE_PSK,
        .cipher_suite = SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256,
        .server_name = "coaps.syntropic.local",
        .psk_identity = (const uint8_t *)"client_1",
        .psk_identity_len = 8,
        .psk_secret = psk,
        .psk_secret_len = sizeof(psk),
    };
    SYN_DTLS_Config srv_psk_cfg = psk_cfg;
    srv_psk_cfg.is_server = true;

    static uint8_t srv_rx[512];
    static uint8_t srv_tx[512];
    SYN_DTLS_Context srv_dtls;
    mock_transport_reset();
    TEST_ASSERT_TRUE(syn_coaps_client_init(&coaps_client, &psk_cfg, &g_transport, dtls_rx,
                                           sizeof(dtls_rx), dtls_tx, sizeof(dtls_tx)));
    TEST_ASSERT_TRUE(syn_dtls_init(&srv_dtls, &srv_psk_cfg, &g_transport, srv_rx, sizeof(srv_rx),
                                   srv_tx, sizeof(srv_tx)));

    /* Step 1: sends ClientHello and returns SYN_OK while awaiting ServerHello */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 2000U));
    TEST_ASSERT_EQUAL_INT(SYN_DTLS_STATE_CLIENT_HELLO_SENT, coaps_client.dtls.state);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(80U, (uint32_t)g_mock_tx_len);

    /* Server processes ClientHello and produces ServerHello + Finished */
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&srv_dtls));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Feed ServerHello + Finished back to client and step -> ESTABLISHED + REGISTERING */
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 2050U));
    TEST_ASSERT_EQUAL_INT(SYN_DTLS_STATE_ESTABLISHED, coaps_client.dtls.state);
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 2100U));
}

void test_lwm2m_task_edge_cases_and_branches(void)
{
    lwm2m_task_test_setup();

    /* Step with NULL client / transport in config */
    SYN_LwM2M_Task broken_task = g_task;
    broken_task.cfg.client = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_step(&broken_task, 1000U));

    broken_task = g_task;
    broken_task.cfg.transport = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_lwm2m_task_step(&broken_task, 1000U));

    /* Update build failure (empty location_path): zero-init upd_msg does not transmit */
    g_client.lifetime_s = 0U;
    g_client.state = SYN_LWM2M_STATE_REGISTERED;
    g_client.last_update_ms = 0U;
    g_client.location_path[0] = '\0';
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 61000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Failed build_notify (unregistered object 8888): zero-init notify_msg does not transmit */
    uint8_t bad_obs_tok[1] = {0x77};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 8888U, 0U, 0, bad_obs_tok, 1U, 0U, 10U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 8888U, 0U, 0));
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 61100U));
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);
    (void)syn_lwm2m_observe_remove(&g_client, bad_obs_tok, 1U);

    /* Lifetime = 0 edge case with valid location_path */
    (void)strcpy(g_client.location_path, "rd/1");
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 61500U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_UPDATING, g_client.state);

    /* Deregister while in UPDATING state */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_deregister(&g_task));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 62000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);

    /* Deregister while already in DEREGISTERED state */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_deregister(&g_task));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Inbound request while in DEREGISTERED state and within backoff window */
    g_task.last_action_ms = 63100U;
    SYN_CoapMsg get_req = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 0x5050};
    uint8_t req_frame[64];
    size_t req_len = syn_coap_serialize(&get_req, NULL, 0U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63100U));
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Unsolicited response in REGISTERED state */
    g_client.state = SYN_LWM2M_STATE_REGISTERED;
    SYN_CoapMsg unsol_resp = {.type = COAP_TYPE_ACK, .code = COAP_RESP_CHANGED, .msg_id = 0x7777};
    req_len = syn_coap_serialize(&unsol_resp, NULL, 0U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63200U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);

    /* Inbound ping / empty message */
    SYN_CoapMsg empty_msg = {.type = COAP_TYPE_CON, .code = 0U, .msg_id = 0x8888};
    req_len = syn_coap_serialize(&empty_msg, NULL, 0U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63300U));

    /* Object hooks with NULL object pointer and zero instances */
    SYN_LwM2M_Object empty_obj = {.id = 9999, .instance_count = 0};
    g_client.objects[0] = NULL;
    g_client.objects[1] = &empty_obj;
    g_task.cfg.client = &g_client;
    /* Step downlink request to trigger hook check */
    get_req.code = COAP_CODE_GET;
    get_req.msg_id = 0x5051;
    SYN_CoapOption path_opt = {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3303", .len = 4};
    req_len = syn_coap_serialize(&get_req, &path_opt, 1U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63400U));

    /* Unhandled client state default branch */
    g_client.state = (SYN_LwM2M_ClientState)99;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 63500U));
}

static bool g_mock_has_data_flag = false;

static bool mock_transport_has_data_cb(const void *ctx)
{
    (void)ctx;
    return g_mock_has_data_flag;
}

static uint32_t decode_coap_uint_opt(const SYN_CoapOption *opt)
{
    uint32_t v = 0U;
    for (size_t i = 0U; i < opt->len; i++) {
        v = (v << 8U) | (uint32_t)opt->val[i];
    }
    return v;
}

void test_lwm2m_task_pt_has_data_yield_and_n14_observe_seq(void)
{
    /* 1. syn_lwm2m_task_pt has_data != NULL && syn_transport_has_data() -> PT_CONTINUE_IF_NEEDED
     *    (syn_lwm2m_task.c:432-435) */
    lwm2m_task_test_setup();

    SYN_Transport async_tr = {
        .send = mock_transport_send,
        .recv = mock_transport_recv,
        .has_data = mock_transport_has_data_cb,
        .ctx = NULL,
    };
    g_task.cfg.transport = &async_tr;
    g_client.transport = &async_tr;

    SYN_Sched sched = {0};
    SYN_Task sched_task = {0};
    sched_task.user_data = &g_task;
    sched_task.sched = &sched;
    sched.running = &sched_task;
    sched.share_contended = true;

    SYN_PT pt;
    PT_INIT(&pt);

    /* Idle has_data == true with empty RX queue must yield PT_IDLE without spinning */
    sched.share_contended = false;
    g_mock_has_data_flag = true;
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_lwm2m_task_pt(&pt, &sched_task));

    /* Consuming an inbound packet while has_data == true and contended yields PT_MORE */
    SYN_CoapMsg ping_msg = {.type = COAP_TYPE_CON, .code = 0U, .msg_id = 0x9001};
    uint8_t ping_frame[16];
    size_t ping_len = syn_coap_serialize(&ping_msg, NULL, 0U, ping_frame, sizeof(ping_frame));
    mock_transport_feed_rx(ping_frame, ping_len);
    sched.share_contended = true;
    g_mock_has_data_flag = true;
    TEST_ASSERT_EQUAL_INT(PT_MORE, syn_lwm2m_task_pt(&pt, &sched_task));

    g_mock_has_data_flag = false;
    TEST_ASSERT_EQUAL_INT(PT_IDLE, syn_lwm2m_task_pt(&pt, &sched_task));

    /* 2. Regression N-14: Observe notifications increment sequence number by 1 per notification
     *    (1, 2, 3 — never double-incremented) */
    test_lwm2m_task_registration_success_flow();

    SYN_CoapMsg obs_req = {0};
    obs_req.type = COAP_TYPE_CON;
    obs_req.code = COAP_CODE_GET;
    obs_req.msg_id = 0x3100;
    obs_req.token_len = 2U;
    obs_req.token[0] = 0xBE;
    obs_req.token[1] = 0xEF;

    uint8_t obs_zero = 0U;
    SYN_CoapOption obs_opts[4] = {
        {.num = COAP_OPT_OBSERVE, .val = &obs_zero, .len = 0U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3303", .len = 4U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"5700", .len = 4U},
    };
    uint8_t req_frame[128];
    size_t req_len = syn_coap_serialize(&obs_req, obs_opts, 4U, req_frame, sizeof(req_frame));
    mock_transport_reset();
    mock_transport_feed_rx(req_frame, req_len);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 5000U));
    TEST_ASSERT_TRUE(g_client.observations[0].active);
    g_client.observations[0].pmin = 0U;
    g_client.observations[0].pmax = 60U;

    for (uint32_t expected_seq = 1U; expected_seq <= 3U; expected_seq++) {
        mock_transport_reset();
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 5000U + expected_seq * 1000U));
        TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

        SYN_CoapMsg notif;
        SYN_CoapOption notif_opts[8];
        size_t notif_opt_cnt = 0U;
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_coap_parse(&notif, notif_opts, 8U, &notif_opt_cnt,
                                                     g_mock_tx_buf, g_mock_tx_len));
        bool found_obs = false;
        for (size_t i = 0U; i < notif_opt_cnt; i++) {
            if (notif_opts[i].num == COAP_OPT_OBSERVE) {
                found_obs = true;
                TEST_ASSERT_EQUAL_UINT32(expected_seq, decode_coap_uint_opt(&notif_opts[i]));
            }
        }
        TEST_ASSERT_TRUE(found_obs);
    }
    TEST_ASSERT_EQUAL_UINT32(3U, g_client.observations[0].seq);
}

void test_lwm2m_task_r4_p05_p06_regressions(void)
{
    /* 1. R4-P06: Packet drop timeout in REGISTERING + mismatched msg_id ignored */
    lwm2m_task_test_setup();

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    uint16_t reg_mid = g_task.pending_msg_id;

    /* Feed spoofed/stale 4.00 response with wrong msg_id -> ignored */
    SYN_CoapMsg wrong_resp = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_BAD_REQ,
        .msg_id = (uint16_t)(reg_mid ^ 0x55AAU),
    };
    uint8_t frame[128];
    size_t flen = syn_coap_serialize(&wrong_resp, NULL, 0U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1500U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_task.retry_count);

    /* Timeout after retry_backoff_ms (2000ms) -> transitions to DEREGISTERED */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(1U, g_task.retry_count);
    TEST_ASSERT_EQUAL_UINT32(3000U, g_task.last_action_ms);

    /* Retry registration after backoff (5000ms) and complete registration */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 5000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    TEST_ASSERT_NOT_EQUAL(reg_mid, g_task.pending_msg_id);

    SYN_CoapMsg ok_resp = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_CREATED,
        .msg_id = g_task.pending_msg_id,
    };
    flen = syn_coap_serialize(&ok_resp, NULL, 0U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 5100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);

    /* 2. R4-P05: Default 60s periodic observe notification & overflow-safe pmax */
    uint8_t obs_zero = 0U;
    SYN_CoapMsg obs_req = {
        .type = COAP_TYPE_CON,
        .code = COAP_CODE_GET,
        .msg_id = 0x4100U,
        .token_len = 2U,
        .token = {0xCA, 0xFE},
    };
    SYN_CoapOption obs_opts[4] = {
        {.num = COAP_OPT_OBSERVE, .val = &obs_zero, .len = 0U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3303", .len = 4U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"5700", .len = 4U},
    };
    flen = syn_coap_serialize(&obs_req, obs_opts, 4U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 6000U));
    TEST_ASSERT_TRUE(g_client.observations[0].active);
    TEST_ASSERT_EQUAL_UINT32(60U, g_client.observations[0].pmax);
    g_client.observations[0].last_notify_ms = 6000U;

    /* Unrelated object change does not fire notification */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3U, 0U, 9));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 65000U));
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* At 60s (66000ms), default pmax=60s periodic notification fires */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 66000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    /* Overflow-safe pmax / pmin (> UINT32_MAX / 1000U) does not wrap modulo 2^32 */
    g_client.observations[0].pmax = 5000000U;
    g_client.observations[0].pmin = 5000000U;
    g_client.observations[0].last_notify_ms = 66000U;
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 72000U));
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* Fill remaining observer slots and verify full table omits COAP_OPT_OBSERVE */
    for (size_t i = 1U; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        uint8_t tok = (uint8_t)(0x20U + i);
        TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 3U, 0U, 9, &tok, 1U, 0U, 120U));
    }
    obs_req.msg_id = 0x4101U;
    obs_req.token[0] = 0x99U;
    obs_req.token[1] = 0x88U;
    flen = syn_coap_serialize(&obs_req, obs_opts, 4U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 73000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    SYN_CoapMsg get_resp;
    SYN_CoapOption get_resp_opts[8];
    size_t get_resp_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_coap_parse(&get_resp, get_resp_opts, 8U, &get_resp_opt_cnt,
                                                 g_mock_tx_buf, g_mock_tx_len));
    for (size_t i = 0U; i < get_resp_opt_cnt; i++) {
        TEST_ASSERT_NOT_EQUAL(COAP_OPT_OBSERVE, get_resp_opts[i].num);
    }

    /* 3. R4-P06: Packet drop timeout in UPDATING + mismatched msg_id ignored */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_trigger_update(&g_task));
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 74000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_UPDATING, g_client.state);
    uint16_t upd_mid = g_task.pending_msg_id;

    /* Wrong msg_id in UPDATING is ignored */
    wrong_resp.code = COAP_RESP_NOT_FOUND;
    wrong_resp.msg_id = (uint16_t)(upd_mid + 1U);
    flen = syn_coap_serialize(&wrong_resp, NULL, 0U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 75000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_UPDATING, g_client.state);

    /* Timeout after retry_backoff_ms (2000ms) -> transitions to DEREGISTERED */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 76000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(1U, g_task.retry_count);
}

void test_r5_n10_lwm2m_task_lifetime_u64_and_dirty_queue(void)
{
    test_lwm2m_task_registration_success_flow();

    /* 1. Large lifetime_s (5368710 * 800 = 4294968000 > UINT32_MAX) saturates to UINT32_MAX
     *    instead of wrapping modulo 2^32 to 704 ms! */
    g_client.lifetime_s = 5368710U;
    g_client.last_update_ms = 1000U;
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 10000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, g_mock_tx_len);

    /* 2. Multi-entry dirty queue delivers ALL resource changes in a single step */
    uint8_t tok_a[1] = {0xA1U};
    uint8_t tok_b[1] = {0xB2U};
    uint8_t tok_inst[1] = {0xC3U};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 3303U, 0U, 5700, tok_a, 1U, 0U, 0U));
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 3U, 0U, 9, tok_b, 1U, 0U, 0U));
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 3303U, 0U, -1, tok_inst, 1U, 0U, 0U));

    /* Queue two distinct resources + duplicate (deduplicated) in one tick */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3U, 0U, 9));
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)g_task.dirty_count);

    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 12000U));
    /* All 3 observers (3303/0/5700, 3/0/9, and instance-level 3303/0/-1) must have fired! */
    TEST_ASSERT_EQUAL_UINT32(1U, g_client.observations[0].seq);
    TEST_ASSERT_EQUAL_UINT32(1U, g_client.observations[1].seq);
    TEST_ASSERT_EQUAL_UINT32(1U, g_client.observations[2].seq);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)g_task.dirty_count);

    /* Whole-instance dirty notification (d->res_id == -1) triggers specific resource observer */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, -1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 13000U));
    TEST_ASSERT_EQUAL_UINT32(2U, g_client.observations[0].seq);
    TEST_ASSERT_EQUAL_UINT32(2U, g_client.observations[2].seq);

    /* Fill dirty_queue to SYN_LWM2M_MAX_DIRTY_QUEUE and verify overflow returns SYN_BUSY */
    for (uint16_t i = 0U; i < SYN_LWM2M_MAX_DIRTY_QUEUE; i++) {
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 9000U + i, 0U, 1));
    }
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_lwm2m_task_notify_changed(&g_task, 9999U, 0U, 1));
}

void test_pro07_lwm2m_task_coaps_pmin_dereg_and_zero_tick(void)
{
    /* 1. Tick 0 registration rejection must not retry-flood on subsequent now_ms == 0 steps */
    lwm2m_task_test_setup();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 0U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);

    SYN_CoapMsg rej_resp = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_BAD_REQ,
        .msg_id = g_task.pending_msg_id,
    };
    uint8_t frame[256];
    size_t flen = syn_coap_serialize(&rej_resp, NULL, 0U, frame, sizeof(frame));
    mock_transport_reset();
    mock_transport_feed_rx(frame, flen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 0U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(1U, g_task.retry_count);

    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 0U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)g_mock_tx_len);

    /* 2. pmin quiet-window latches dirty change and fires once pmin expires; deregister clears */
    test_lwm2m_task_registration_success_flow();
    uint8_t obs_tok[2] = {0x12U, 0x34U};
    TEST_ASSERT_TRUE(syn_lwm2m_observe_add(&g_client, 3303U, 0U, 5700, obs_tok, 2U, 5U, 60U));
    g_client.observations[0].last_notify_ms = 2000U;

    /* Change arrives during pmin quiet window (3000ms < 2000ms + 5000ms) */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_notify_changed(&g_task, 3303U, 0U, 5700));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 3000U));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)g_mock_tx_len);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)g_task.dirty_count);
    TEST_ASSERT_TRUE(g_task.pending_notify[0]);

    /* Once pmin expires (7000ms), latched notification fires without a second notify_changed */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 7000U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    TEST_ASSERT_FALSE(g_task.pending_notify[0]);

    /* Deregistration clears active observations and pending_notify state */
    g_task.pending_notify[0] = true;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_deregister(&g_task));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 8000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_DEREGISTERED, g_client.state);
    TEST_ASSERT_FALSE(g_client.observations[0].active);
    TEST_ASSERT_FALSE(g_task.pending_notify[0]);

    /* 3. DTLS CoAPs end-to-end encrypted registration and downlink dispatch (no cleartext leak) */
    lwm2m_task_test_setup();
    static uint8_t s_cli_rx[512];
    static uint8_t s_cli_tx[512];
    static uint8_t s_srv_rx[512];
    static uint8_t s_srv_tx[512];
    static SYN_CoapsClient s_coaps;
    static SYN_DTLS_Context s_srv_dtls;
    const uint8_t psk[32] = {0x42U};
    SYN_DTLS_Config cli_dtls_cfg = {
        .mode = SYN_DTLS_AUTH_MODE_PSK,
        .cipher_suite = SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256,
        .server_name = "coaps.syntropic.local",
        .psk_identity = (const uint8_t *)"lwm2m_ep",
        .psk_identity_len = 8U,
        .psk_secret = psk,
        .psk_secret_len = sizeof(psk),
    };
    SYN_DTLS_Config srv_dtls_cfg = cli_dtls_cfg;
    srv_dtls_cfg.is_server = true;

    TEST_ASSERT_TRUE(syn_coaps_client_init(&s_coaps, &cli_dtls_cfg, &g_transport, s_cli_rx,
                                           sizeof(s_cli_rx), s_cli_tx, sizeof(s_cli_tx)));
    TEST_ASSERT_TRUE(syn_dtls_init(&s_srv_dtls, &srv_dtls_cfg, &g_transport, s_srv_rx,
                                   sizeof(s_srv_rx), s_srv_tx, sizeof(s_srv_tx)));

    /* Re-init task with coaps_client bound upfront */
    SYN_LwM2M_TaskConfig coaps_task_cfg = g_task.cfg;
    coaps_task_cfg.coaps_client = &s_coaps;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_init(&g_task, &coaps_task_cfg));

    /* Complete 3-flight DTLS 1.3 PSK handshake */
    mock_transport_reset();
    TEST_ASSERT_TRUE(syn_coaps_client_handshake(&s_coaps));
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&s_srv_dtls));
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_TRUE(syn_coaps_client_handshake(&s_coaps));
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_handshake(&s_srv_dtls));
    TEST_ASSERT_EQUAL_INT(SYN_DTLS_STATE_ESTABLISHED, s_coaps.dtls.state);
    TEST_ASSERT_EQUAL_INT(SYN_DTLS_STATE_ESTABLISHED, s_srv_dtls.state);

    /* Step 1: Registration POST is DTLS-encrypted (not raw CoAP ver=1 header 0x40..0x48) */
    mock_transport_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1000U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERING, g_client.state);
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    TEST_ASSERT_NOT_EQUAL(0x42U, g_mock_tx_buf[0]);

    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    uint8_t dec_buf[256];
    size_t dec_len = 0U;
    TEST_ASSERT_TRUE(syn_dtls_recv(&s_srv_dtls, dec_buf, sizeof(dec_buf), &dec_len));
    SYN_CoapMsg reg_coap = {0};
    SYN_CoapOption reg_opts[8];
    size_t reg_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_coap_parse(&reg_coap, reg_opts, 8U, &reg_opt_cnt, dec_buf, dec_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_POST, reg_coap.code);

    /* Server sends DTLS-encrypted 2.01 Created */
    SYN_CoapMsg created_resp = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_CREATED,
        .msg_id = reg_coap.msg_id,
    };
    flen = syn_coap_serialize(&created_resp, NULL, 0U, frame, sizeof(frame));
    mock_transport_reset();
    TEST_ASSERT_TRUE(syn_dtls_send(&s_srv_dtls, frame, flen));
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1100U));
    TEST_ASSERT_EQUAL_INT(SYN_LWM2M_STATE_REGISTERED, g_client.state);

    /* Server sends DTLS-encrypted GET /3/0/0; client replies over DTLS */
    SYN_CoapMsg dl_get = {.type = COAP_TYPE_CON, .code = COAP_CODE_GET, .msg_id = 0x7070U};
    SYN_CoapOption dl_opts[3] = {
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"3", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
        {.num = COAP_OPT_URI_PATH, .val = (const uint8_t *)"0", .len = 1U},
    };
    flen = syn_coap_serialize(&dl_get, dl_opts, 3U, frame, sizeof(frame));
    mock_transport_reset();
    TEST_ASSERT_TRUE(syn_dtls_send(&s_srv_dtls, frame, flen));
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    g_mock_tx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_step(&g_task, 1200U));
    TEST_ASSERT_TRUE(g_mock_tx_len > 0U);
    mock_transport_feed_rx(g_mock_tx_buf, g_mock_tx_len);
    TEST_ASSERT_TRUE(syn_dtls_recv(&s_srv_dtls, dec_buf, sizeof(dec_buf), &dec_len));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_coap_parse(&reg_coap, reg_opts, 8U, &reg_opt_cnt, dec_buf, dec_len));
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CONTENT, reg_coap.code);
}

/* ── Runner Entry Point ─────────────────────────────────────────────────── */

void run_lwm2m_task_tests(void)
{
    RUN_TEST(test_lwm2m_task_init_null_and_validation);
    RUN_TEST(test_lwm2m_task_registration_success_flow);
    RUN_TEST(test_lwm2m_task_registration_failure_and_retry_backoff);
    RUN_TEST(test_lwm2m_task_lifetime_update_and_force_trigger);
    RUN_TEST(test_lwm2m_task_downlink_requests_and_execute_hooks);
    RUN_TEST(test_lwm2m_task_observe_and_notify_scheduler);
    RUN_TEST(test_lwm2m_task_graceful_deregistration);
    RUN_TEST(test_lwm2m_task_scheduler_protothread_integration);
    RUN_TEST(test_lwm2m_task_dtls_coaps_integration);
    RUN_TEST(test_lwm2m_task_edge_cases_and_branches);
    RUN_TEST(test_lwm2m_task_pt_has_data_yield_and_n14_observe_seq);
    RUN_TEST(test_lwm2m_task_r4_p05_p06_regressions);
    RUN_TEST(test_r5_n10_lwm2m_task_lifetime_u64_and_dirty_queue);
    RUN_TEST(test_pro07_lwm2m_task_coaps_pmin_dereg_and_zero_tick);
}
