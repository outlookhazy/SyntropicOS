/**
 * @file test_kwp2000.c
 * @brief Unit tests for ISO 14230-3 KWP2000 Server Protocol Stack.
 */

#include "syntropic/proto/syn_kwp2000.h"
#include "unity/unity.h"

#include <string.h>

static SYN_KWP2000_Server g_kwp_server;
static uint32_t g_dummy_vin_data[4] = {0x12345678, 0x9ABCDEF0, 0x11223344, 0x55667788};
static uint16_t g_dummy_rpm = 2500;

void test_kwp2000_init_and_null_checks(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t req[8];
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_init(NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kwp2000_register_lid(NULL, 0x01, &g_dummy_rpm, 2, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kwp2000_register_lid(&g_kwp_server, 0x01, NULL, 2, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kwp2000_register_cid(NULL, 0xF190, &g_dummy_rpm, 2, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_set_session_handler(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_set_reset_handler(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_set_security_handler(NULL, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_set_routine_handler(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kwp2000_set_memory_handler(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kwp2000_process_request(NULL, req, 1, resp, sizeof(resp), &resp_len));
}

void test_kwp2000_diagnostic_session_control(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t req[] = {0x10, 0x86}; /* StartDiagnosticSession (Extended) */
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(2, resp_len);
    TEST_ASSERT_EQUAL(0x50, resp[0]);
    TEST_ASSERT_EQUAL(0x86, resp[1]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_EXTENDED, g_kwp_server.current_session);
}

void test_kwp2000_read_data_by_local_identifier(void)
{
    syn_kwp2000_init(&g_kwp_server);
    syn_kwp2000_register_lid(&g_kwp_server, 0x01, &g_dummy_rpm, sizeof(g_dummy_rpm), false);

    uint8_t req[] = {0x21, 0x01}; /* ReadDataByLocalIdentifier LID 0x01 */
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(4, resp_len);
    TEST_ASSERT_EQUAL(0x61, resp[0]);
    TEST_ASSERT_EQUAL(0x01, resp[1]);

    uint16_t read_rpm = 0;
    memcpy(&read_rpm, &resp[2], 2);
    TEST_ASSERT_EQUAL(2500, read_rpm);
}

void test_kwp2000_read_data_by_common_identifier(void)
{
    syn_kwp2000_init(&g_kwp_server);
    syn_kwp2000_register_cid(&g_kwp_server, 0xF190, g_dummy_vin_data, sizeof(g_dummy_vin_data),
                             false);

    uint8_t req[] = {0x22, 0xF1, 0x90}; /* ReadDataByCommonIdentifier CID 0xF190 */
    uint8_t resp[32];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(3 + sizeof(g_dummy_vin_data), resp_len);
    TEST_ASSERT_EQUAL(0x62, resp[0]);
    TEST_ASSERT_EQUAL(0xF1, resp[1]);
    TEST_ASSERT_EQUAL(0x90, resp[2]);
}

void test_kwp2000_security_access_seed_key(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t req_seed[] = {0x27, 0x01};
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_seed, sizeof(req_seed),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x67, resp[0]);
    TEST_ASSERT_EQUAL(0x01, resp[1]);

    uint8_t req_key[] = {0x27, 0x02, 0xC5, 0xC9};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_key, sizeof(req_key),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x67, resp[0]);
    TEST_ASSERT_EQUAL(0x02, resp[1]);
    TEST_ASSERT_TRUE(g_kwp_server.security_unlocked);
}

static bool g_session_cb_called = false;
static bool test_kwp_session_cb(SYN_KWP2000_Session from, SYN_KWP2000_Session to, void *ctx)
{
    (void)from;
    (void)to;
    (void)ctx;
    g_session_cb_called = true;
    return true;
}

static bool g_reset_cb_called = false;
static void test_kwp_reset_cb(uint8_t reset_type, void *ctx)
{
    (void)reset_type;
    (void)ctx;
    g_reset_cb_called = true;
}

static bool test_kwp_seed_cb(uint8_t level, uint8_t *seed_buf, uint16_t *seed_len, void *ctx)
{
    (void)level;
    (void)ctx;
    seed_buf[0] = 0x12;
    seed_buf[1] = 0x34;
    *seed_len = 2;
    return true;
}

static bool test_kwp_key_cb(uint8_t level, const uint8_t *key, uint16_t key_len, void *ctx)
{
    (void)level;
    (void)ctx;
    return (key_len >= 2 && key[0] == 0x12 && key[1] == 0x34);
}

static bool test_kwp_routine_cb(uint8_t routine_id, const uint8_t *in_data, uint16_t in_len,
                                uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                void *ctx)
{
    (void)in_data;
    (void)in_len;
    (void)ctx;
    if (routine_id == 0x01 && max_out_len >= 1) {
        out_buf[0] = 0xAA;
        if (out_len)
            *out_len = 1;
        return true;
    }
    return false;
}

static uint8_t g_mem_buf[256];
static bool test_kwp_mem_cb(bool is_write, uint32_t address, uint32_t size, uint8_t *data_buf,
                            void *ctx)
{
    (void)ctx;
    if (is_write && address < sizeof(g_mem_buf)) {
        memcpy(&g_mem_buf[address], data_buf, size);
        return true;
    }
    return false;
}

void test_kwp2000_ecu_reset_and_callbacks(void)
{
    syn_kwp2000_init(&g_kwp_server);
    g_reset_cb_called = false;
    syn_kwp2000_set_reset_handler(&g_kwp_server, test_kwp_reset_cb, NULL);
    g_kwp_server.reset_wait_ms = 50;

    uint8_t req[] = {0x11, 0x01};
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(2, resp_len);
    TEST_ASSERT_EQUAL(0x51, resp[0]);
    TEST_ASSERT_EQUAL(0x01, resp[1]);

    syn_kwp2000_tick(NULL, 10);
    syn_kwp2000_tick(&g_kwp_server, 20);
    TEST_ASSERT_FALSE(g_reset_cb_called);
    TEST_ASSERT_EQUAL(30, g_kwp_server.reset_wait_ms);

    syn_kwp2000_tick(&g_kwp_server, 40);
    TEST_ASSERT_TRUE(g_reset_cb_called);
}

void test_kwp2000_session_and_security_callbacks(void)
{
    syn_kwp2000_init(&g_kwp_server);
    g_session_cb_called = false;
    syn_kwp2000_set_session_handler(&g_kwp_server, test_kwp_session_cb, NULL);
    syn_kwp2000_set_security_handler(&g_kwp_server, test_kwp_seed_cb, test_kwp_key_cb, NULL);

    uint8_t req_sess[] = {0x10, 0x85};
    uint8_t resp[16];
    uint16_t resp_len = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_sess, sizeof(req_sess),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_TRUE(g_session_cb_called);

    uint8_t req_seed[] = {0x27, 0x01};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_seed, sizeof(req_seed),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x67, resp[0]);
    TEST_ASSERT_EQUAL(0x12, resp[2]);
    TEST_ASSERT_EQUAL(0x34, resp[3]);

    uint8_t req_key[] = {0x27, 0x02, 0x12, 0x34};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_key, sizeof(req_key),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x67, resp[0]);
    TEST_ASSERT_TRUE(g_kwp_server.security_unlocked);
}

void test_kwp2000_routine_control(void)
{
    syn_kwp2000_init(&g_kwp_server);
    syn_kwp2000_set_routine_handler(&g_kwp_server, test_kwp_routine_cb, NULL);
    g_kwp_server.security_unlocked = true;

    uint8_t req[] = {0x31, 0x01, 0x55};
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(3, resp_len);
    TEST_ASSERT_EQUAL(0x71, resp[0]);
    TEST_ASSERT_EQUAL(0x01, resp[1]);
    TEST_ASSERT_EQUAL(0xAA, resp[2]);
}

void test_kwp2000_download_and_transfer_data(void)
{
    syn_kwp2000_init(&g_kwp_server);
    syn_kwp2000_set_memory_handler(&g_kwp_server, test_kwp_mem_cb, NULL);
    g_kwp_server.security_unlocked = true;

    uint8_t req_dl[] = {0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_dl, sizeof(req_dl),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x74, resp[0]);

    uint8_t req_tx[] = {0x36, 0x01, 0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_tx, sizeof(req_tx),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x76, resp[0]);
    TEST_ASSERT_EQUAL(0x01, resp[1]);

    uint8_t req_exit[] = {0x37};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_exit, sizeof(req_exit),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x77, resp[0]);
}

void test_kwp2000_tester_present_and_nrcs(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t req_tp[] = {0x3E};
    uint8_t resp[16];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_tp, sizeof(req_tp),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7E, resp[0]);

    uint8_t req_bad_len[] = {0x10};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_bad_len, sizeof(req_bad_len),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7F, resp[0]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
}

void test_kwp2000_edge_cases_and_overflow(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t dummy = 0;

    /* Fill LID table to max */
    for (uint8_t i = 0; i < SYN_KWP2000_MAX_LIDS; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_lid(&g_kwp_server, i + 1, &dummy, 1, false));
    }
    /* Re-register existing LID */
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_lid(&g_kwp_server, 1, &dummy, 1, true));
    /* Overflow LID table */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kwp2000_register_lid(&g_kwp_server, 0xFE, &dummy, 1, false));

    /* Fill CID table to max */
    for (uint16_t j = 0; j < SYN_KWP2000_MAX_CIDS; j++) {
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_kwp2000_register_cid(&g_kwp_server, 0x1000 + j, &dummy, 1, false));
    }
    /* Re-register existing CID */
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_cid(&g_kwp_server, 0x1000, &dummy, 1, true));
    /* Overflow CID table */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kwp2000_register_cid(&g_kwp_server, 0xFFFF, &dummy, 1, false));
}

static bool test_kwp_session_reject_cb(SYN_KWP2000_Session from, SYN_KWP2000_Session to, void *ctx)
{
    (void)from;
    (void)to;
    (void)ctx;
    return false;
}

static bool test_kwp_routine_reject_cb(uint8_t routine_id, const uint8_t *in_data, uint16_t in_len,
                                       uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                       void *ctx)
{
    (void)routine_id;
    (void)in_data;
    (void)in_len;
    (void)out_buf;
    (void)max_out_len;
    (void)out_len;
    (void)ctx;
    return false;
}

static bool test_kwp_mem_reject_cb(bool is_write, uint32_t address, uint32_t size,
                                   uint8_t *data_buf, void *ctx)
{
    (void)is_write;
    (void)address;
    (void)size;
    (void)data_buf;
    (void)ctx;
    return false;
}

void test_kwp2000_rejections_and_nrc_branches(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t resp[16];
    uint16_t resp_len = 0;

    /* Rejected Session Transition */
    syn_kwp2000_set_session_handler(&g_kwp_server, test_kwp_session_reject_cb, NULL);
    uint8_t req_sess[] = {0x10, 0x85};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_sess, sizeof(req_sess),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7F, resp[0]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Rejected Routine Control */
    g_kwp_server.security_unlocked = true;
    syn_kwp2000_set_routine_handler(&g_kwp_server, test_kwp_routine_reject_cb, NULL);
    uint8_t req_rt[] = {0x31, 0x05};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_rt, sizeof(req_rt),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7F, resp[0]);

    /* Rejected Memory Write */
    syn_kwp2000_set_memory_handler(&g_kwp_server, test_kwp_mem_reject_cb, NULL);
    g_kwp_server.is_downloading = true;
    g_kwp_server.expected_block_seq = 1U;
    g_kwp_server.download_size = 16U;
    uint8_t req_tx[] = {0x36, 0x01, 0xAA};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_tx, sizeof(req_tx),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7F, resp[0]);

    /* LID / CID Not Found */
    uint8_t req_bad_lid[] = {0x21, 0xFF};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_bad_lid, sizeof(req_bad_lid),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t req_bad_cid[] = {0x22, 0xFF, 0xFF};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_bad_cid, sizeof(req_bad_cid),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
}

static bool test_kwp_seed_reject_cb(uint8_t level, uint8_t *seed_buf, uint16_t *seed_len, void *ctx)
{
    (void)level;
    (void)seed_buf;
    (void)seed_len;
    (void)ctx;
    return false;
}

void test_kwp2000_remaining_coverage_branches(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t resp[16];
    uint16_t resp_len = 0;

    /* Empty request with valid buffer */
    uint8_t dummy_req[1] = {0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, dummy_req, 0, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Short requests for all SIDs */
    uint8_t req_short_10[] = {0x10};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_10, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_11[] = {0x11};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_11, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_21[] = {0x21};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_21, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_22[] = {0x22, 0xF1};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_22, 2, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_27[] = {0x27};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_27, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_27_key[] = {0x27, 0x02};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_27_key, 2, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_31[] = {0x31};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_31, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_34[] = {0x34, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_34, 3, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t req_short_36[] = {0x36};
    g_kwp_server.is_downloading = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_short_36, 1, resp,
                                                          sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Invalid Session Subfunction */
    uint8_t req_bad_sess[] = {0x10, 0xFF};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_bad_sess, sizeof(req_bad_sess),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* Invalid Reset Type */
    uint8_t req_bad_reset[] = {0x11, 0xFF};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_bad_reset,
                                                          sizeof(req_bad_reset), resp, sizeof(resp),
                                                          &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* Response Buffer Overflow for LID */
    syn_kwp2000_register_lid(&g_kwp_server, 0x01, &g_dummy_rpm, sizeof(g_dummy_rpm), false);
    uint8_t req_lid[] = {0x21, 0x01};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_lid, sizeof(req_lid),
                                                          resp, 3, &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Response Buffer Overflow for CID */
    syn_kwp2000_register_cid(&g_kwp_server, 0xF190, g_dummy_vin_data, sizeof(g_dummy_vin_data),
                             false);
    uint8_t req_cid[] = {0x22, 0xF1, 0x90};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_cid, sizeof(req_cid),
                                                          resp, 5, &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Seed Callback Rejection */
    syn_kwp2000_set_security_handler(&g_kwp_server, test_kwp_seed_reject_cb, NULL, NULL);
    uint8_t req_seed[] = {0x27, 0x01};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_seed, sizeof(req_seed),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Invalid Key without callback */
    syn_kwp2000_init(&g_kwp_server);
    uint8_t req_bad_key[] = {0x27, 0x02, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_bad_key, sizeof(req_bad_key),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INVALID_KEY, resp[2]);
    /* Unknown SID default branch */
    uint8_t req_unknown[] = {0x99};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kwp2000_process_request(&g_kwp_server, req_unknown, sizeof(req_unknown),
                                                  resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL(0x7F, resp[0]);
    TEST_ASSERT_EQUAL(0x99, resp[1]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED, resp[2]);
}

void test_kwp2000_security_access_resp_overflow(void)
{
    syn_kwp2000_init(&g_kwp_server);
    /* Enter extended session first */
    uint8_t session_req[] = {0x10, SYN_KWP2000_SESSION_EXTENDED};
    uint8_t resp[4]; /* Intentionally small: only 4 bytes */
    uint16_t resp_len = 0;
    syn_kwp2000_process_request(&g_kwp_server, session_req, sizeof(session_req), resp, sizeof(resp),
                                &resp_len);

    /* SecurityAccess seed request (SID 0x27, subfunc 0x01)
     * Default seed is 2 bytes → response is 4 bytes (SID+0x40, subfunc, seed[2]).
     * With max_resp_len=4, this fits. But a custom seed_cb returning 16 bytes
     * would produce 18-byte response → overflow. Use default seed (2 bytes)
     * with max_resp_len=3 to trigger the guard. */
    uint8_t sec_req[] = {0x27, 0x01};
    uint8_t tiny_resp[3];
    resp_len = 0;
    SYN_Status st = syn_kwp2000_process_request(&g_kwp_server, sec_req, sizeof(sec_req), tiny_resp,
                                                sizeof(tiny_resp), &resp_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    /* Must return NRC instead of overflowing the 3-byte buffer */
    TEST_ASSERT_EQUAL(0x7F, tiny_resp[0]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, tiny_resp[2]);
}

static bool test_kwp_routine_overflow_cb(uint8_t routine_id, const uint8_t *in_data,
                                         uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                         uint16_t *out_len, void *ctx)
{
    (void)routine_id;
    (void)in_data;
    (void)in_len;
    (void)max_out_len;
    (void)ctx;
    out_buf[0] = 0xAA;
    out_buf[1] = 0xBB;
    out_buf[2] = 0xCC;
    out_buf[3] = 0xDD;
    *out_len = 4;
    return true;
}

void test_kwp2000_routine_control_resp_overflow(void)
{
    syn_kwp2000_init(&g_kwp_server);
    g_kwp_server.security_unlocked = true;
    syn_kwp2000_set_routine_handler(&g_kwp_server, test_kwp_routine_overflow_cb, NULL);

    uint8_t req[] = {0x31, 0x01};
    uint8_t tiny_resp[4]; /* 2 + 4 = 6 bytes needed, only 4 available */
    uint16_t resp_len = 0;
    SYN_Status st = syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), tiny_resp,
                                                sizeof(tiny_resp), &resp_len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(0x7F, tiny_resp[0]);
    TEST_ASSERT_EQUAL(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, tiny_resp[2]);
}

void test_r5_a09_writes_security_transfer_bounds_and_s3_refresh(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t lid_val[2] = {0x11, 0x22};
    uint8_t lid_ro[2] = {0x33, 0x44};
    uint8_t cid_val[3] = {0xAA, 0xBB, 0xCC};
    uint8_t cid_ro[3] = {0xDD, 0xEE, 0xFF};
    uint8_t resp[32];
    uint16_t resp_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_lid(&g_kwp_server, 0x10U, lid_val, 2U, true));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_lid(&g_kwp_server, 0x11U, lid_ro, 2U, false));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_cid(&g_kwp_server, 0xF100U, cid_val, 3U, true));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_register_cid(&g_kwp_server, 0xF101U, cid_ro, 3U, false));

    /* Short, unknown, read-only, wrong-length, and security-locked LID/CID writes */
    uint8_t short_lid[] = {0x3B, 0x10};
    syn_kwp2000_process_request(&g_kwp_server, short_lid, sizeof(short_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t unk_lid[] = {0x3B, 0x99, 0x01, 0x02};
    syn_kwp2000_process_request(&g_kwp_server, unk_lid, sizeof(unk_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t ro_lid[] = {0x3B, 0x11, 0x01, 0x02};
    syn_kwp2000_process_request(&g_kwp_server, ro_lid, sizeof(ro_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t len_lid[] = {0x3B, 0x10, 0x01};
    syn_kwp2000_process_request(&g_kwp_server, len_lid, sizeof(len_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t wr_lid[] = {0x3B, 0x10, 0x77, 0x88};
    syn_kwp2000_process_request(&g_kwp_server, wr_lid, sizeof(wr_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    uint8_t short_cid[] = {0x2E, 0xF1, 0x00};
    syn_kwp2000_process_request(&g_kwp_server, short_cid, sizeof(short_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t unk_cid[] = {0x2E, 0xF9, 0x99, 0x01};
    syn_kwp2000_process_request(&g_kwp_server, unk_cid, sizeof(unk_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t ro_cid[] = {0x2E, 0xF1, 0x01, 0x01, 0x02, 0x03};
    syn_kwp2000_process_request(&g_kwp_server, ro_cid, sizeof(ro_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t len_cid[] = {0x2E, 0xF1, 0x00, 0x01, 0x02};
    syn_kwp2000_process_request(&g_kwp_server, len_cid, sizeof(len_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    uint8_t wr_cid[] = {0x2E, 0xF1, 0x00, 0x01, 0x02, 0x03};
    syn_kwp2000_process_request(&g_kwp_server, wr_cid, sizeof(wr_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* Unprivileged 0x31 routine succeeds while locked; privileged routine_cb rejects with 0x33 */
    uint8_t req_rt[] = {0x31, 0x01};
    syn_kwp2000_process_request(&g_kwp_server, req_rt, sizeof(req_rt), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x71, resp[0]);

    syn_kwp2000_set_routine_handler(&g_kwp_server, test_kwp_routine_reject_cb, NULL);
    syn_kwp2000_process_request(&g_kwp_server, req_rt, sizeof(req_rt), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp[2]);
    syn_kwp2000_set_routine_handler(&g_kwp_server, NULL, NULL);

    /* Locked 0x34 must fail with NRC 0x33 */
    uint8_t req_dl5[] = {0x34, 0x00, 0x00, 0x00, 0x00};
    syn_kwp2000_process_request(&g_kwp_server, req_dl5, sizeof(req_dl5), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* Enter extended session, unlock security, verify S3 refresh on non-TesterPresent traffic */
    uint8_t sess_ext[] = {0x10, SYN_KWP2000_SESSION_EXTENDED};
    syn_kwp2000_process_request(&g_kwp_server, sess_ext, sizeof(sess_ext), resp, sizeof(resp),
                                &resp_len);
    uint8_t key_ok[] = {0x27, 0x02, 0xC5, 0xC9};
    syn_kwp2000_process_request(&g_kwp_server, key_ok, sizeof(key_ok), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_TRUE(g_kwp_server.security_unlocked);

    syn_kwp2000_tick(&g_kwp_server, 4000U);
    TEST_ASSERT_EQUAL_UINT32(4000U, g_kwp_server.s3_timer_ms);

    /* Valid 0x3B and 0x2E writes refresh s3_timer_ms to 0 and mutate target buffers */
    syn_kwp2000_process_request(&g_kwp_server, wr_lid, sizeof(wr_lid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_UINT32(0U, g_kwp_server.s3_timer_ms);
    TEST_ASSERT_EQUAL_HEX8(0x7B, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x77, lid_val[0]);
    TEST_ASSERT_EQUAL_HEX8(0x88, lid_val[1]);

    syn_kwp2000_process_request(&g_kwp_server, wr_cid, sizeof(wr_cid), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x6E, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, cid_val[2]);

    /* Routine without callback succeeds with 2-byte response when unlocked */
    syn_kwp2000_process_request(&g_kwp_server, req_rt, sizeof(req_rt), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x71, resp[0]);
    TEST_ASSERT_EQUAL_UINT16(2U, resp_len);

    /* RequestDownload (5-byte unbounded and 9-byte bounded) + 0x36 sequence & size checks */
    syn_kwp2000_process_request(&g_kwp_server, req_dl5, sizeof(req_dl5), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x74, resp[0]);
    uint8_t tx_ok1[] = {0x36, 0x01, 0xAA, 0xBB};
    syn_kwp2000_process_request(&g_kwp_server, tx_ok1, sizeof(tx_ok1), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x76, resp[0]);

    uint8_t req_dl4b[] = {0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    syn_kwp2000_process_request(&g_kwp_server, req_dl4b, sizeof(req_dl4b), resp, sizeof(resp),
                                &resp_len);
    uint8_t tx_bad_seq[] = {0x36, 0x02, 0x01};
    syn_kwp2000_process_request(&g_kwp_server, tx_bad_seq, sizeof(tx_bad_seq), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    uint8_t tx_over[] = {0x36, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05};
    syn_kwp2000_process_request(&g_kwp_server, tx_over, sizeof(tx_over), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 0x10 session transition relocks security_unlocked and aborts active download */
    syn_kwp2000_process_request(&g_kwp_server, sess_ext, sizeof(sess_ext), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_FALSE(g_kwp_server.security_unlocked);
    TEST_ASSERT_FALSE(g_kwp_server.is_downloading);

    /* Not downloading 0x36 check + reset tick without reset_cb */
    syn_kwp2000_process_request(&g_kwp_server, tx_ok1, sizeof(tx_ok1), resp, sizeof(resp),
                                &resp_len);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
    g_kwp_server.pending_reset_type = 0x01U;
    syn_kwp2000_tick(&g_kwp_server, 10U);
    TEST_ASSERT_EQUAL_UINT8(0U, g_kwp_server.pending_reset_type);
}

void test_kwp2000_s3_session_timeout(void)
{
    syn_kwp2000_init(&g_kwp_server);

    /* Enter extended session */
    uint8_t req[] = {0x10, SYN_KWP2000_SESSION_EXTENDED};
    uint8_t resp[16];
    uint16_t resp_len = 0;
    syn_kwp2000_process_request(&g_kwp_server, req, sizeof(req), resp, sizeof(resp), &resp_len);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_EXTENDED, g_kwp_server.current_session);

    /* Unlock security */
    g_kwp_server.security_unlocked = true;

    /* Tick 4999ms — should NOT timeout */
    syn_kwp2000_tick(&g_kwp_server, 4999U);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_EXTENDED, g_kwp_server.current_session);
    TEST_ASSERT_TRUE(g_kwp_server.security_unlocked);

    /* Tick 1 more ms — total 5000ms → S3 timeout must fire */
    syn_kwp2000_tick(&g_kwp_server, 1U);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_DEFAULT, g_kwp_server.current_session);
    TEST_ASSERT_FALSE(g_kwp_server.security_unlocked);

    /* Already in DEFAULT — subsequent ticks should be no-ops */
    syn_kwp2000_tick(&g_kwp_server, 10000U);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_DEFAULT, g_kwp_server.current_session);
}

static bool test_kwp_pro03_overlong_seed_cb(uint8_t level, uint8_t *seed_buf, uint16_t *seed_len,
                                            void *ctx)
{
    (void)level;
    (void)seed_buf;
    (void)ctx;
    *seed_len = 20U;
    return true;
}

static bool test_kwp_pro03_overlong_routine_cb(uint8_t routine_id, const uint8_t *in_data,
                                               uint16_t in_len, uint8_t *out_buf,
                                               uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)routine_id;
    (void)in_data;
    (void)in_len;
    (void)out_buf;
    (void)max_out_len;
    (void)ctx;
    *out_len = 80U;
    return true;
}

void test_kwp2000_pro03_bounds_and_transfer_guards(void)
{
    syn_kwp2000_init(&g_kwp_server);
    uint8_t resp[128];
    uint16_t resp_len = 0U;

    /* 1. SecurityAccess sub_func == 0x00 rejected with NRC 0x12 */
    const uint8_t req_sec0[2] = {0x27U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_sec0, sizeof(req_sec0),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 2. SecurityAccess seed_len > sizeof(seed_buf) (16) rejected before memcpy */
    syn_kwp2000_set_security_handler(&g_kwp_server, test_kwp_pro03_overlong_seed_cb, NULL, NULL);
    const uint8_t req_seed[2] = {0x27U, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_seed, sizeof(req_seed),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 3. RoutineControl out_len > sizeof(routine_out) (64) rejected before memcpy */
    g_kwp_server.security_unlocked = true;
    syn_kwp2000_set_routine_handler(&g_kwp_server, test_kwp_pro03_overlong_routine_cb, NULL);
    const uint8_t req_rt[2] = {0x31U, 0x01U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_rt, sizeof(req_rt),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 4. RequestDownload partial size lengths (6..8 bytes) rejected with NRC 0x12 */
    const uint8_t req_dl_partial[8] = {0x34U, 0x00U, 0x00U, 0x10U, 0x00U, 0x00U, 0x00U, 0x10U};
    for (uint16_t len = 6U; len <= 8U; len++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_dl_partial, len,
                                                              resp, sizeof(resp), &resp_len));
        TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
    }

    /* 5. RequestTransferExit (0x37) rejected with NRC 0x22 when download not active */
    const uint8_t req_exit[1] = {0x37U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_exit, sizeof(req_exit),
                                                          resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 6. max_resp_len < 3 guard on 0x10 and 0x3E + s3_timer_ms saturation */
    const uint8_t req_10[2] = {0x10U, SYN_KWP2000_SESSION_EXTENDED};
    const uint8_t req_3e[1] = {0x3EU};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kwp2000_process_request(&g_kwp_server, req_10, sizeof(req_10), resp, 2U, &resp_len));
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kwp2000_process_request(&g_kwp_server, req_3e, sizeof(req_3e), resp, 2U, &resp_len));

    TEST_ASSERT_EQUAL(SYN_OK, syn_kwp2000_process_request(&g_kwp_server, req_10, sizeof(req_10),
                                                          resp, sizeof(resp), &resp_len));
    syn_kwp2000_tick(&g_kwp_server, 100U);
    syn_kwp2000_tick(&g_kwp_server, UINT32_MAX);
    TEST_ASSERT_EQUAL(SYN_KWP2000_SESSION_DEFAULT, g_kwp_server.current_session);
}

void run_kwp2000_tests(void)
{
    RUN_TEST(test_kwp2000_init_and_null_checks);
    RUN_TEST(test_kwp2000_diagnostic_session_control);
    RUN_TEST(test_kwp2000_read_data_by_local_identifier);
    RUN_TEST(test_kwp2000_read_data_by_common_identifier);
    RUN_TEST(test_kwp2000_security_access_seed_key);
    RUN_TEST(test_kwp2000_ecu_reset_and_callbacks);
    RUN_TEST(test_kwp2000_session_and_security_callbacks);
    RUN_TEST(test_kwp2000_routine_control);
    RUN_TEST(test_kwp2000_download_and_transfer_data);
    RUN_TEST(test_kwp2000_tester_present_and_nrcs);
    RUN_TEST(test_kwp2000_edge_cases_and_overflow);
    RUN_TEST(test_kwp2000_rejections_and_nrc_branches);
    RUN_TEST(test_kwp2000_remaining_coverage_branches);
    RUN_TEST(test_kwp2000_security_access_resp_overflow);
    RUN_TEST(test_kwp2000_routine_control_resp_overflow);
    RUN_TEST(test_kwp2000_s3_session_timeout);
    RUN_TEST(test_r5_a09_writes_security_transfer_bounds_and_s3_refresh);
    RUN_TEST(test_kwp2000_pro03_bounds_and_transfer_guards);
}
