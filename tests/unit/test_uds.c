/**
 * @file test_uds.c
 * @brief Unit tests for ISO 14229 UDS server protocol implementation.
 */

#include "syntropic/crypto/syn_aes.h"
#include "syntropic/proto/syn_isotp.h"
#include "syntropic/proto/syn_uds.h"
#include "syntropic/util/syn_pack.h"
#include "unity/unity.h"

#include <string.h>

static SYN_UDS_Server g_uds;
static uint8_t g_test_did_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};

static void test_uds_init_and_sessions(void)
{
    TEST_ASSERT_FALSE(syn_uds_init(NULL));
    TEST_ASSERT_TRUE(syn_uds_init(&g_uds));
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_DEFAULT, g_uds.session);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, g_uds.security_state);

    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* Short request -> Incorrect message length */
    req[0] = SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Invalid subfunction */
    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* Extended Session (Allowed from DEFAULT) */
    req[1] = SYN_UDS_SESSION_EXTENDED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SESSION_EXTENDED, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_EXTENDED, g_uds.session);
    TEST_ASSERT_EQUAL_UINT32(5000, g_uds.s3_timer_ms);

    /* Programming Session (Allowed from EXTENDED) */
    req[1] = SYN_UDS_SESSION_PROGRAMMING;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SESSION_PROGRAMMING, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_PROGRAMMING, g_uds.session);

    /* Extended Session (Allowed from PROGRAMMING per ISO 14229-1 default) */
    req[1] = SYN_UDS_SESSION_EXTENDED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SESSION_EXTENDED, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_EXTENDED, g_uds.session);

    /* Default Session resets security lock and S3 timer */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    req[1] = SYN_UDS_SESSION_DEFAULT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_DEFAULT, g_uds.session);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT32(0, g_uds.s3_timer_ms);

    /* Programming Session (Allowed directly from DEFAULT per ISO 14229-1 default) */
    req[1] = SYN_UDS_SESSION_PROGRAMMING;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SESSION_PROGRAMMING, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_PROGRAMMING, g_uds.session);

    /* Safety System Session (0x04) */
    req[1] = SYN_UDS_SESSION_SAFETY_SYSTEM;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SESSION_SAFETY_SYSTEM, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_SAFETY_SYSTEM, g_uds.session);
}

static bool test_session_transition_policy(SYN_UDS_Session from, SYN_UDS_Session to, void *ctx)
{
    (void)ctx;
    /* Custom rule: Default -> Programming directly is disallowed */
    if ((from == SYN_UDS_SESSION_DEFAULT) && (to == SYN_UDS_SESSION_PROGRAMMING)) {
        return false;
    }
    return true;
}

static void test_uds_session_transition_policy(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_DEFAULT};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    syn_uds_set_session_transition_handler(&g_uds, test_session_transition_policy, NULL);

    /* DEFAULT -> PROGRAMMING should be rejected with NRC 0x22 */
    req[1] = SYN_UDS_SESSION_PROGRAMMING;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* DEFAULT -> EXTENDED is allowed */
    req[1] = SYN_UDS_SESSION_EXTENDED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);

    /* EXTENDED -> PROGRAMMING is allowed */
    req[1] = SYN_UDS_SESSION_PROGRAMMING;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);

    syn_uds_set_session_transition_handler(&g_uds, NULL, NULL);
    syn_uds_set_session_transition_handler(NULL, NULL, NULL);
}

static void test_uds_s3_timer_tick(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* Transition to EXTENDED session */
    req[0] = SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    req[1] = SYN_UDS_SESSION_EXTENDED;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_EXTENDED, g_uds.session);

    /* Advance S3 timer by 2000ms -> timer remains active */
    syn_uds_tick(&g_uds, 2000);
    TEST_ASSERT_EQUAL_UINT32(3000, g_uds.s3_timer_ms);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_EXTENDED, g_uds.session);

    /* TesterPresent resets S3 timer back to 5000ms */
    req[0] = SYN_UDS_SID_TESTER_PRESENT;
    req[1] = 0x00;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_UINT32(5000, g_uds.s3_timer_ms);

    /* Advance S3 timer past 5000ms -> expires, drops to DEFAULT and locks security */
    syn_uds_tick(&g_uds, 5000);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_DEFAULT, g_uds.session);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT32(0, g_uds.s3_timer_ms);

    /* Tick when already DEFAULT or NULL is a no-op */
    syn_uds_tick(&g_uds, 1000);
    syn_uds_tick(NULL, 1000);
}

static void test_uds_security_access(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* Immediate Request Seed on power-on succeeds (delay timer = 0 on boot) */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_SEED_SENT, g_uds.security_state);

    /* Send Key (Invalid key #1) */
    req[1] = 0x02;
    req[2] = 0x00;
    req[3] = 0x00;
    req[4] = 0x00;
    req[5] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INVALID_KEY, resp[2]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, g_uds.security_state);

    /* Seed #2 + Invalid key #2 */
    req[1] = 0x01;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02;
    syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);

    /* Seed #3 + Invalid key #3 -> Exceeds max attempts (3) returning NRC 0x36 */
    req[1] = 0x01;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS, resp[2]);

    /* Subsequent Request Seed during 10s lockout fails with NRC 0x37 */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED, resp[2]);

    /* Tick 1s then remaining 9s to expire lockout timer (covers line 109 decrement) */
    syn_uds_tick(&g_uds, 1000);
    syn_uds_tick(&g_uds, 9000);

    /* Request Seed and unlock with correct key */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);

    uint32_t correct_key = g_uds.current_seed ^ 0xA5A5A5A5U;
    req[1] = 0x02;
    syn_poke_u32(correct_key, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT8(1, syn_uds_get_security_level(&g_uds));

    /* Repeat Request Seed cycle when already unlocked */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_SEED_SENT, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT8(0, syn_uds_get_security_level(&g_uds));

    /* Sending valid key again unlocks successfully */
    req[1] = 0x02;
    syn_poke_u32(correct_key, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT8(1, syn_uds_get_security_level(&g_uds));

    /* Re-initialize server and set delay timer to test NRC 0x37 */
    syn_uds_init(&g_uds);
    g_uds.security_delay_timer_ms = 10000;

    /* Request Seed during active 10s lockout fails with NRC 0x37 */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED, resp[2]);

    /* Expire 10s lockout timer */
    syn_uds_tick(&g_uds, 10000);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);

    /* Send Key (Correct expected key = seed ^ 0xA5A5A5A5) */
    uint32_t expected_key = g_uds.current_seed ^ 0xA5A5A5A5U;
    req[1] = 0x02;
    req[2] = (uint8_t)(expected_key >> 24);
    req[3] = (uint8_t)(expected_key >> 16);
    req[4] = (uint8_t)(expected_key >> 8);
    req[5] = (uint8_t)(expected_key);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT8(0, g_uds.security_error_count);

    /* Send Key when seed not sent -> Request sequence error (NRC 0x24) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp[2]);

    /* Programming session mode transition bypasses power-on delay timer */
    syn_uds_init(&g_uds);
    uint8_t sess_ext[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_EXTENDED};
    syn_uds_process_request(&g_uds, sess_ext, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t sess_prog[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_PROGRAMMING};
    syn_uds_process_request(&g_uds, sess_prog, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);

    /* Invalid subfunction */
    req[1] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);
}

static void test_uds_security_access_aes128(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[64] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* Test null parameter guards */
    TEST_ASSERT_FALSE(syn_uds_enable_aes128_security(NULL, NULL));
    TEST_ASSERT_FALSE(syn_uds_enable_aes128_security(&g_uds, NULL));
    TEST_ASSERT_FALSE(syn_uds_enable_aes128_security(NULL, (const uint8_t *)"1234567890123456"));
    TEST_ASSERT_FALSE(syn_uds_disable_aes128_security(NULL));
    TEST_ASSERT_FALSE(syn_uds_set_security_seed_bytes(NULL, NULL));
    TEST_ASSERT_FALSE(syn_uds_set_security_seed_bytes(&g_uds, NULL));

    /* Disable AES-128 security mode check */
    TEST_ASSERT_TRUE(syn_uds_disable_aes128_security(&g_uds));

    /* Enable AES-128 security mode with secret key */
    const uint8_t aes_key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    const uint8_t seed_16[16] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    TEST_ASSERT_TRUE(syn_uds_enable_aes128_security(&g_uds, aes_key));
    TEST_ASSERT_TRUE(syn_uds_set_security_seed_bytes(&g_uds, seed_16));

    /* Expire power-on delay timer */
    syn_uds_tick(&g_uds, 10000);

    /* Request Seed (0x27 0x01) -> Returns 18 bytes (header + sub + 16-byte seed) */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(18, resp_len);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &resp[2], 16);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_SEED_SENT, g_uds.security_state);

    /* Send Key with short payload (< 18 bytes) -> NRC 0x13 */
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 10, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Send Key (Invalid AES Key #1) -> NRC 0x35 */
    req[1] = 0x02;
    memset(&req[2], 0xAA, 16);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INVALID_KEY, resp[2]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, g_uds.security_state);

    /* Request Seed #2 + Invalid Key #2 */
    req[1] = 0x01;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02;
    syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);

    /* Request Seed #3 + Invalid Key #3 -> Exceeds max attempts (3) returning NRC 0x36 */
    req[1] = 0x01;
    syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS, resp[2]);

    /* Request Seed during active 10s lockout -> NRC 0x37 */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED, resp[2]);

    /* Expire 10s lockout */
    syn_uds_tick(&g_uds, 10000);

    /* Request Seed and compute valid AES-128 key */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);

    SYN_AES_Context aes_ctx;
    syn_aes_init(&aes_ctx, aes_key, 16U);
    uint8_t valid_key[16];
    syn_aes_encrypt_block(&aes_ctx, seed_16, valid_key);

    req[1] = 0x02;
    memcpy(&req[2], valid_key, 16);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, g_uds.security_state);
    TEST_ASSERT_EQUAL_UINT8(0, g_uds.security_error_count);

    /* Request Seed when already unlocked -> returns active seed bytes */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &resp[2], 16);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_SEED_SENT, g_uds.security_state);

    /* Sending valid key when unlocked unlocks successfully */
    req[1] = 0x02;
    memcpy(&req[2], valid_key, 16);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, g_uds.security_state);

    /* Verify master seed template current_seed_bytes was NOT corrupted or zeroed */
    TEST_ASSERT_EQUAL_MEMORY(seed_16, g_uds.current_seed_bytes, 16);

    /* Lock security state via DEFAULT session reset and verify cycle N works cleanly */
    g_uds.session = SYN_UDS_SESSION_DEFAULT;
    g_uds.security_state = SYN_UDS_SECURITY_LOCKED;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &resp[2], 16);

    /* Send Key when seed not sent in AES128 mode -> Request sequence error (NRC 0x24) */
    g_uds.security_state = SYN_UDS_SECURITY_LOCKED;
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp[2]);

    /* Invalid subfunction in AES128 mode -> NRC 0x12 */
    req[1] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* Disable AES-128 security mode -> Revert to XOR mode */
    TEST_ASSERT_TRUE(syn_uds_disable_aes128_security(&g_uds));

    /* Invalid subfunction in standard XOR mode -> NRC 0x12 */
    req[1] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);
}

static void test_uds_did_read_write(void)
{
    g_test_did_data[0] = 0xAA;
    g_test_did_data[1] = 0xBB;
    g_test_did_data[2] = 0xCC;
    g_test_did_data[3] = 0xDD;
    syn_uds_init(&g_uds);
    TEST_ASSERT_FALSE(syn_uds_register_did(NULL, 0x1234, g_test_did_data, 4, true));
    TEST_ASSERT_TRUE(syn_uds_register_did(&g_uds, 0x1234, g_test_did_data, 4, true));

    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* Read DID 0x1234 */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0x12;
    req[2] = 0x34;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x62, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x34, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, resp[3]);
    TEST_ASSERT_EQUAL_HEX8(0xDD, resp[6]);

    /* Read non-existent DID -> Request out of range */
    req[1] = 0x99;
    req[2] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* Write DID when locked -> Security access denied */
    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    req[1] = 0x12;
    req[2] = 0x34;
    req[3] = 0x11;
    req[4] = 0x22;
    req[5] = 0x33;
    req[6] = 0x44;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 7, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* SecurityAccess short requests */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Unlock security */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;

    /* Write DID when unlocked -> Success */
    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    req[1] = 0x12;
    req[2] = 0x34;
    req[3] = 0x11;
    req[4] = 0x22;
    req[5] = 0x33;
    req[6] = 0x44;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 7, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6E, resp[0]);

    /* Write DID with extra trailing bytes -> NRC 0x13 IncorrectMessageLength */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 10, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Write non-existent DID -> Request out of range */
    req[1] = 0x99;
    req[2] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 7, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
    /* Read-only DID write attempt */
    syn_uds_register_did(&g_uds, 0x5555, g_test_did_data, 4, false);
    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    req[1] = 0x55;
    req[2] = 0x55;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 7, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Response too long for small buffer */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0x12;
    req[2] = 0x34;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&g_uds, req, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);
}

static void test_uds_ecu_reset_routine_tester_present(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* ECUReset short req vs invalid subfunction */
    req[0] = SYN_UDS_SID_ECU_RESET;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* ECUReset Hard reset */
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, syn_uds_get_pending_reset(&g_uds));
    syn_uds_clear_pending_reset(&g_uds);
    TEST_ASSERT_EQUAL_HEX8(0x00, syn_uds_get_pending_reset(&g_uds));

    /* ECUReset 0x02 keyOffOnReset */
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, syn_uds_get_pending_reset(&g_uds));
    syn_uds_clear_pending_reset(&g_uds);

    /* ECUReset 0x03 softReset */
    req[1] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x03, syn_uds_get_pending_reset(&g_uds));
    syn_uds_clear_pending_reset(&g_uds);

    /* ECUReset 0x04 enableRapidPowerShutDown */
    req[1] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x04, syn_uds_get_pending_reset(&g_uds));
    syn_uds_clear_pending_reset(&g_uds);

    /* ECUReset 0x05 disableRapidPowerShutDown */
    req[1] = 0x05;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x05, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x05, syn_uds_get_pending_reset(&g_uds));
    syn_uds_clear_pending_reset(&g_uds);

    /* RoutineControl short req */
    req[0] = SYN_UDS_SID_ROUTINE_CONTROL;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[1] = 0x01;
    req[2] = 0x02;
    req[3] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x71, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);

    /* TesterPresent short req */
    req[0] = SYN_UDS_SID_TESTER_PRESENT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[1] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7E, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[1]);

    /* RDBI & WDBI short req */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Unsupported SID */
    req[0] = 0xBA;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SERVICE_NOT_SUPPORTED, resp[2]);
}

static void test_uds_bounds_and_null_checks(void)
{
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    TEST_ASSERT_FALSE(syn_uds_process_request(NULL, req, 2, resp, sizeof(resp), &resp_len,
                                              SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_FALSE(syn_uds_process_request(&g_uds, NULL, 2, resp, sizeof(resp), &resp_len,
                                              SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_FALSE(syn_uds_process_request(&g_uds, req, 0, resp, sizeof(resp), &resp_len,
                                              SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_FALSE(syn_uds_process_request(&g_uds, req, 2, NULL, sizeof(resp), &resp_len,
                                              SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_FALSE(
        syn_uds_process_request(&g_uds, req, 2, resp, 2, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_FALSE(
        syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), NULL, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0U, syn_uds_get_pending_reset(NULL));

    /* Register DID overflow */
    syn_uds_init(&g_uds);
    for (size_t i = 0; i < SYN_UDS_MAX_DIDS; i++) {
        TEST_ASSERT_TRUE(syn_uds_register_did(&g_uds, (uint16_t)i, g_test_did_data, 4, true));
    }
    TEST_ASSERT_FALSE(syn_uds_register_did(&g_uds, 0x9999, g_test_did_data, 4, true));
}

static bool mock_comm_control_handler(SYN_UDS_CommControlType control_type, uint8_t comm_type,
                                      void *ctx)
{
    (void)ctx;
    if (control_type == SYN_UDS_COMM_DISABLE_RX_AND_TX && comm_type == 0xFF) {
        return false; /* Simulate rejection */
    }
    return true;
}

static void test_uds_communication_control(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* 1. By default, CommunicationControl succeeds in DEFAULT session */
    req[0] = SYN_UDS_SID_COMMUNICATION_CONTROL;
    req[1] = SYN_UDS_COMM_ENABLE_RX_AND_TX;
    req[2] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x68, resp[0]);

    /* Restrict CommunicationControl to EXTENDED session -> NRC 0x7F in DEFAULT session */
    TEST_ASSERT_TRUE(syn_uds_set_service_session_mask(&g_uds, 0x28, SYN_UDS_SESSION_MASK_EXTENDED));
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp[2]);

    /* Switch to EXTENDED session */
    uint8_t sess_ext[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_EXTENDED};
    syn_uds_process_request(&g_uds, sess_ext, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    /* 2. Short length check (< 3) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 3. Invalid subfunction (> 5) -> NRC 0x12 */
    req[1] = 0x06;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 4. Zero comm_type -> NRC 0x31 */
    req[1] = SYN_UDS_COMM_ENABLE_RX_AND_TX;
    req[2] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 5. Successful CommunicationControl subfunctions 0x00..0x05 */
    for (uint8_t sub = 0; sub <= 5; sub++) {
        req[1] = sub;
        req[2] = 0x01;
        TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x68, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(sub, resp[1]);
        TEST_ASSERT_EQUAL(sub, g_uds.comm_control_state);
        TEST_ASSERT_EQUAL_HEX8(0x01, g_uds.comm_type);
    }

    /* 6. Custom handler registration & failure case */
    TEST_ASSERT_FALSE(syn_uds_register_comm_control(NULL, mock_comm_control_handler, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_comm_control(&g_uds, mock_comm_control_handler, NULL));

    req[1] = SYN_UDS_COMM_DISABLE_RX_AND_TX;
    req[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 7. S3 timeout reverts comm control state back to ENABLE_RX_AND_TX (0x00) */
    syn_uds_tick(&g_uds, 5000);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_DEFAULT, g_uds.session);
    TEST_ASSERT_EQUAL(SYN_UDS_COMM_ENABLE_RX_AND_TX, g_uds.comm_control_state);
}

static bool mock_timing_handler(SYN_UDS_AccessTimingType timing_type, uint16_t *p2_max_ms,
                                uint16_t *p2_star_max_10ms, void *ctx)
{
    (void)p2_star_max_10ms;
    (void)ctx;
    if (timing_type == SYN_UDS_TIMING_SET_TO_GIVEN) {
        if (*p2_max_ms > 1000U) {
            return false; /* Reject values out of allowed range */
        }
    }
    return true;
}

static void test_uds_access_timing_parameter(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* 1. Null server check */
    TEST_ASSERT_FALSE(syn_uds_register_access_timing(NULL, mock_timing_handler, NULL));

    /* 2. Short request -> NRC 0x13 */
    req[0] = SYN_UDS_SID_ACCESS_TIMING_PARAMETER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 3. Invalid subfunction -> NRC 0x12 */
    req[1] = 0x05;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 4. Subfunction 0x01: readExtendedTimingParameterSet */
    req[1] = SYN_UDS_TIMING_READ_EXTENDED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC3, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_TIMING_READ_EXTENDED, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(6, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x32, resp[3]); /* 50 ms */
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[4]);
    TEST_ASSERT_EQUAL_HEX8(0xF4, resp[5]); /* 500 (5000 ms) */

    /* 5. Subfunction 0x01 wrong length -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 6. Subfunction 0x04: setTimingParametersToGivenValues (P2=100ms, P2*=1000) */
    req[1] = SYN_UDS_TIMING_SET_TO_GIVEN;
    req[2] = 0x00;
    req[3] = 0x64; /* 100 ms */
    req[4] = 0x03;
    req[5] = 0xE8; /* 1000 (10000 ms) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC3, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_TIMING_SET_TO_GIVEN, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(2, resp_len);
    TEST_ASSERT_EQUAL_UINT16(100, g_uds.active_p2_max_ms);
    TEST_ASSERT_EQUAL_UINT16(1000, g_uds.active_p2_star_max_10ms);

    /* 7. Subfunction 0x03: readCurrentlyActiveTimingParameterSet */
    req[1] = SYN_UDS_TIMING_READ_ACTIVE;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC3, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_TIMING_READ_ACTIVE, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(6, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x64, resp[3]); /* 100 ms */
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[4]);
    TEST_ASSERT_EQUAL_HEX8(0xE8, resp[5]); /* 1000 */

    /* 8. Subfunction 0x02: setTimingParametersToDefaultValues */
    req[1] = SYN_UDS_TIMING_SET_TO_DEFAULT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC3, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_TIMING_SET_TO_DEFAULT, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(2, resp_len);
    TEST_ASSERT_EQUAL_UINT16(50, g_uds.active_p2_max_ms);
    TEST_ASSERT_EQUAL_UINT16(500, g_uds.active_p2_star_max_10ms);

    /* 9. Custom callback rejection */
    TEST_ASSERT_TRUE(syn_uds_register_access_timing(&g_uds, mock_timing_handler, NULL));
    req[1] = SYN_UDS_TIMING_SET_TO_GIVEN;
    req[2] = 0x05;
    req[3] = 0x00; /* P2 = 1280 ms > 1000 -> Handler rejects */
    req[4] = 0x03;
    req[5] = 0xE8;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
}

static bool mock_secured_data_handler(const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf,
                                      uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)ctx;
    (void)max_out_len;
    if (in_len == 0 || in_data[0] == 0xFF) {
        return false; /* Rejection case */
    }
    /* Simple XOR cipher for testing */
    for (uint16_t i = 0; i < in_len; i++) {
        out_buf[i] = in_data[i] ^ 0x5AU;
    }
    *out_len = in_len;
    return true;
}

static void test_uds_secured_data_transmission(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* 1. Null server check */
    TEST_ASSERT_FALSE(syn_uds_register_secured_data(NULL, mock_secured_data_handler, NULL));

    /* 2. Short request (req_len < 2) -> NRC 0x13 */
    req[0] = SYN_UDS_SID_SECURED_DATA_TRANSMISSION;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 3. Locked state -> NRC 0x33 (Security Access Denied) */
    req[1] = 0x12;
    req[2] = 0x34;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* 4. Unlock security & test default echo response (no callback) */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC4, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x34, resp[2]);
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);

    /* 5. Custom handler registration & encrypted payload transformation */
    TEST_ASSERT_TRUE(syn_uds_register_secured_data(&g_uds, mock_secured_data_handler, NULL));
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC4, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12 ^ 0x5A, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x34 ^ 0x5A, resp[2]);

    /* 6. Handler rejection -> NRC 0x22 */
    req[1] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
}

static void test_uds_extended_sids(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* ClearDiagnosticInformation (0x14) */
    req[0] = SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION;
    req[1] = 0xFF;
    req[2] = 0xFF;
    req[3] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);

    /* ReadDTCInformation (0x19) */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x02;
    req[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);

    /* ControlDTCSetting (0x85) */
    req[0] = SYN_UDS_SID_CONTROL_DTC_SETTING;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC5, resp[0]);

    /* ResponseOnEvent (0x86) */
    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    req[1] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC6, resp[0]);

    /* Transfer Data Services (0x34, 0x35, 0x36, 0x37) */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00;
    req[2] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x74, resp[0]);

    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76, resp[0]);

    req[0] = SYN_UDS_SID_REQUEST_TRANSFER_EXIT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x77, resp[0]);
}

static bool mock_memory_cb(bool is_write, uint32_t address, uint32_t size, uint8_t *data_buf,
                           void *ctx)
{
    (void)address;
    (void)size;
    (void)ctx;
    if (is_write) {
        data_buf[0] = 0xAA;
    } else {
        data_buf[0] = 0xBB;
    }
    return true;
}

static bool mock_auth_cb(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                         uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)subfunction;
    (void)in_data;
    (void)in_len;
    (void)max_out_len;
    (void)ctx;
    out_buf[0] = 0x99;
    *out_len = 1;
    return true;
}

static bool mock_file_transfer_cb(uint8_t mode, const char *file_path, uint16_t path_len,
                                  uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                  void *ctx)
{
    (void)mode;
    (void)file_path;
    (void)path_len;
    (void)max_out_len;
    (void)ctx;
    out_buf[0] = 0x10;
    *out_len = 1;
    return true;
}

static void test_uds_complete_27_sids(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[32] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* 1. Null registration checks */
    TEST_ASSERT_FALSE(syn_uds_register_memory_handler(NULL, mock_memory_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_auth_handler(NULL, mock_auth_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_file_transfer(NULL, mock_file_transfer_cb, NULL));

    TEST_ASSERT_TRUE(syn_uds_register_memory_handler(&g_uds, mock_memory_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_auth_handler(&g_uds, mock_auth_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_file_transfer(&g_uds, mock_file_transfer_cb, NULL));

    /* 2. ReadMemoryByAddress (0x23) */
    req[0] = SYN_UDS_SID_READ_MEMORY_BY_ADDRESS;
    req[1] = 0x12; /* 2-byte addr, 1-byte size */
    req[2] = 0x10;
    req[3] = 0x00;
    req[4] = 0x04;
    /* Locked state -> NRC 0x33 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x63, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, resp[1]);

    /* Register DID 0xF190 for Scaling (0x24) & IOControl (0x2F) tests */
    static uint8_t f190_val[4] = {0x11, 0x22, 0x33, 0x44};
    syn_uds_register_did(&g_uds, 0xF190, f190_val, 4, true);

    /* 3. ReadScalingDataByIdentifier (0x24) */
    req[0] = SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x64, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0xF1, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x90, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[3]);

    /* 4. Authentication (0x29) */
    req[0] = SYN_UDS_SID_AUTHENTICATION;
    req[1] = 0x00; /* deAuthenticate */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x69, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x99, resp[2]);

    /* 5. ReadDataByPeriodicIdentifier (0x2A) */
    req[0] = SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER;
    req[1] = 0x01; /* Fast mode */
    req[2] = 0xE0; /* Periodic DID */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6A, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0xE0, resp[1]);

    /* 6. DynamicallyDefineDataIdentifier (0x2C) */
    req[0] = SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER;
    req[1] = 0x01; /* defineByIdentifier */
    req[2] = 0xF2;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6C, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0xF2, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[3]);

    /* 7. InputOutputControlByIdentifier (0x2F) */
    req[0] = SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    req[3] = 0x03; /* Short term adjustment */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0xF1, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x90, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[3]);

    /* 8. RequestFileTransfer (0x38) */
    req[0] = SYN_UDS_SID_REQUEST_FILE_TRANSFER;
    req[1] = 0x01; /* AddFile */
    req[2] = '/';
    req[3] = 'a';
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x78, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x10, resp[2]);

    /* 9. WriteMemoryByAddress (0x3D) */
    req[0] = SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS;
    req[1] = 0x11; /* 1-byte addr, 1-byte size */
    req[2] = 0x20;
    req[3] = 0x01;
    req[4] = 0x55;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7D, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x11, resp[1]);

    /* 9b. 64-bit ALFID (>4B): zero-extended valid vs non-zero upper byte overflow (NRC 0x31) */
    uint8_t dl64_ok[12] = {SYN_UDS_SID_REQUEST_DOWNLOAD,
                           0x00,
                           0x18, /* 1-byte size, 8-byte addr */
                           0x00,
                           0x00,
                           0x00,
                           0x00,
                           0x20,
                           0x00,
                           0x10,
                           0x00,
                           0x10};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, dl64_ok, sizeof(dl64_ok), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x74, resp[0]);
    TEST_ASSERT_EQUAL_HEX32(0x20001000U, g_uds.transfer_address);
    TEST_ASSERT_EQUAL_HEX32(0x10U, g_uds.transfer_size);

    dl64_ok[6] = 0x01U; /* 0x0000000120001000 overflows uint32_t */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, dl64_ok, sizeof(dl64_ok), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t rm64_ovf[11] = {SYN_UDS_SID_READ_MEMORY_BY_ADDRESS,
                            0x18,
                            0x00,
                            0x00,
                            0x00,
                            0x01,
                            0x20,
                            0x00,
                            0x10,
                            0x00,
                            0x01};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, rm64_ovf, sizeof(rm64_ovf), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    uint8_t wm64_ovf[12] = {SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS,
                            0x18,
                            0x00,
                            0x00,
                            0x00,
                            0x01,
                            0x20,
                            0x00,
                            0x10,
                            0x00,
                            0x01,
                            0xAA};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, wm64_ovf, sizeof(wm64_ovf), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 10. LinkControl (0x87) */
    req[0] = SYN_UDS_SID_LINK_CONTROL;
    req[1] = 0x01; /* verifyBaudrateTransitionWithFixedBaudrate */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC7, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
}

static bool mock_dtc_custom_cb(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                               uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)in_data;
    (void)in_len;
    (void)max_out_len;
    (void)ctx;
    if (subfunction == 0x01 || subfunction == 0x02 || subfunction == 0x03 || subfunction == 0x04 ||
        subfunction == 0x07 || subfunction == 0x08 || subfunction == 0x0A || subfunction == 0x14 ||
        subfunction == 0x15 || subfunction == 0x55) {
        out_buf[0] = 0x01;
        out_buf[1] = 0x23;
        out_buf[2] = 0x45;
        out_buf[3] = 0x01;
        *out_len = 4;
        return true;
    }
    return false;
}

static bool mock_dtc_reject_cb(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                               uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)subfunction;
    (void)in_data;
    (void)in_len;
    (void)out_buf;
    (void)max_out_len;
    (void)out_len;
    (void)ctx;
    return false;
}

static void test_uds_read_dtc_information_subfunctions(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    /* Register test DTCs */
    TEST_ASSERT_TRUE(
        syn_uds_register_dtc(&server, 0x012345U, 0x09U, 0x60U)); /* testFailed | confirmedDTC */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x023456U, 0x01U, 0x80U)); /* testFailed */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x034567U, 0x00U, 0x10U)); /* clear */

    uint8_t req[16] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* 1. reportNumberOfDTCByStatusMask (0x01) */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x01;
    req[2] = 0x01; /* Mask: testFailed */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_DTC_FORMAT_ISO14229_1, resp[3]);
    TEST_ASSERT_EQUAL_UINT16(2, ((uint16_t)resp[4] << 8) | resp[5]);

    /* 2. reportDTCByStatusMask (0x02) */
    req[1] = 0x02;
    req[2] = 0x01; /* Mask: testFailed */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(11, resp_len); /* 3 + 4 * 2 */

    /* 3. reportSupportedDTC (0x0A) */
    req[1] = 0x0A;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0A, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(15, resp_len); /* 3 + 4 * 3 */

    /* 4. reportFirstTestFailedDTC (0x0B) */
    req[1] = 0x0B;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0B, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[3]);
    TEST_ASSERT_EQUAL_HEX8(0x23, resp[4]);

    /* 5. reportFirstConfirmedDTC (0x0C) */
    req[1] = 0x0C;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0C, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[3]);
    TEST_ASSERT_EQUAL_HEX8(0x23, resp[4]);

    /* 6. reportNumberOfDTCBySeverityMaskRecord (0x07) */
    req[1] = 0x07;
    req[2] = 0x60; /* Severity mask */
    req[3] = 0x01; /* Status mask */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x07, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(1, ((uint16_t)resp[4] << 8) | resp[5]);

    /* 7. reportDTCBySeverityMaskRecord (0x08) */
    req[1] = 0x08;
    req[2] = 0x60;
    req[3] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x08, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(9, resp_len); /* 3 + 6 * 1 */

    /* 8. Custom DTC callback (0x01, 0x02, 0x03, 0x07, 0x0A, 0x15) */
    syn_uds_register_dtc_handler(&server, mock_dtc_custom_cb, NULL);
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);

    req[1] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[2]);
    TEST_ASSERT_EQUAL_HEX8(0x23, resp[3]);

    req[1] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04, resp[1]);

    req[1] = 0x07;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x07, resp[1]);

    req[1] = 0x08;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x08, resp[1]);

    req[1] = 0x0A;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0A, resp[1]);

    req[1] = 0x14;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x14, resp[1]);

    req[1] = 0x15;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x15, resp[1]);

    req[1] = 0x55;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x55, resp[1]);

    /* Callback rejection tests -> NRC 0x22 */
    syn_uds_register_dtc_handler(&server, mock_dtc_reject_cb, NULL);
    uint8_t reject_subs[] = {0x01, 0x02, 0x04, 0x07, 0x08, 0x0A, 0x14, 0x55};
    for (size_t i = 0; i < sizeof(reject_subs); i++) {
        req[1] = reject_subs[i];
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
    }

    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* 9. Subfunction length & overflow mismatch tests -> NRC 0x13 / 0x38 */
    syn_uds_register_dtc_handler(&server, NULL, NULL);
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x01; /* Expected 3 bytes, send 2 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    req[1] = 0x03; /* Expected 2 bytes, send 3 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x14; /* Expected 2 bytes, send 3 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x16; /* Expected 3 bytes (sub + recNum), send 2 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x04; /* Send 6 bytes, but max_resp_len = 5 -> NRC 0x14 */
    req[2] = 0x01;
    req[3] = 0x23;
    req[4] = 0x45;
    req[5] = 0x01;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 6, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x0B; /* Expected 2 bytes, send 3 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x0A; /* Expected 2 bytes, send 3 bytes */
    req[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
}

static void test_uds_stateful_data_transfer_sequence(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;

    uint8_t req[16] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* TransferData without RequestDownload -> NRC 0x24 (requestSequenceError) */
    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp[2]);

    /* RequestTransferExit without active transfer -> NRC 0x24 (requestSequenceError) */
    req[0] = SYN_UDS_SID_REQUEST_TRANSFER_EXIT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp[2]);

    /* RequestDownload (0x34): ALFID 0x11 (1-byte addr 0x20, 1-byte size 0x10) */
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00; /* DFI */
    req[2] = 0x11; /* ALFID */
    req[3] = 0x20; /* Addr */
    req[4] = 0x10; /* Size = 16 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x74, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x20, resp[1]);
    TEST_ASSERT_EQUAL(SYN_UDS_TRANSFER_DOWNLOAD, server.transfer_state);
    TEST_ASSERT_EQUAL_UINT32(0x20, server.transfer_address);
    TEST_ASSERT_EQUAL_UINT32(0x10, server.transfer_size);

    /* TransferData (0x36): Sequence 0x02 instead of expected 0x01 -> NRC 0x73 */
    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    req[1] = 0x02; /* Out-of-order sequence counter */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER, resp[2]);

    /* TransferData (0x36): Sequence 0x01 (Valid 4 bytes payload) */
    req[1] = 0x01;
    req[2] = 0xDE;
    req[3] = 0xAD;
    req[4] = 0xBE;
    req[5] = 0xEF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_UINT32(4, server.transfer_bytes_processed);

    /* RequestTransferExit (0x37): Completes download sequence and resets state to IDLE */
    req[0] = SYN_UDS_SID_REQUEST_TRANSFER_EXIT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x77, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_TRANSFER_IDLE, server.transfer_state);
}

static bool mock_failing_memory_cb(bool is_write, uint32_t address, uint32_t size,
                                   uint8_t *data_buf, void *ctx)
{
    (void)is_write;
    (void)address;
    (void)size;
    (void)data_buf;
    (void)ctx;
    return false;
}

static bool mock_failing_timing_cb(SYN_UDS_AccessTimingType timing_type, uint16_t *p2_max_ms,
                                   uint16_t *p2_star_max_10ms, void *ctx)
{
    (void)timing_type;
    (void)p2_max_ms;
    (void)p2_star_max_10ms;
    (void)ctx;
    return false;
}

static void test_uds_spec_nrc_and_edge_cases(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;

    uint8_t req[32] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* 1. ReadDTCInformation subfunction 0x14 (Fault detection counter) */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x112233U, 0x01U, 0x20U));
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x14;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x14, resp[1]);

    /* 2. ReadDTCInformation subfunctions 0x0D & 0x0E (Most recent failed / confirmed) */
    req[1] = 0x0D;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0D, resp[1]);

    req[1] = 0x0E;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0E, resp[1]);

    /* 3. AccessTimingParameter failing callback -> NRC 0x22 */
    syn_uds_register_access_timing(&server, mock_failing_timing_cb, NULL);
    req[0] = SYN_UDS_SID_ACCESS_TIMING_PARAMETER;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 4. RequestDownload length check -> NRC 0x13 */
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00;
    req[2] = 0x22; /* 2-byte addr, 2-byte size -> requires 7 bytes */
    req[3] = 0x10;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 5. TransferData failing memory_cb -> NRC 0x72 */
    syn_uds_register_memory_handler(&server, mock_failing_memory_cb, NULL);
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00;
    req[2] = 0x11; /* 1-byte addr, 1-byte size */
    req[3] = 0x10;
    req[4] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x74, resp[0]);

    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    req[1] = 0x01;
    req[2] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE, resp[2]);
}

static void test_uds_upload_sequence_and_nrc_handling(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;

    uint8_t req[32] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* 1. RequestUpload (0x35) -> TransferData upload mode -> RequestTransferExit */
    req[0] = SYN_UDS_SID_REQUEST_UPLOAD;
    req[1] = 0x00; /* DFI */
    req[2] = 0x11; /* ALFID: 1-byte addr, 1-byte size */
    req[3] = 0x10; /* Addr */
    req[4] = 0x08; /* Size = 8 bytes */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x75, resp[0]);
    TEST_ASSERT_EQUAL(SYN_UDS_TRANSFER_UPLOAD, server.transfer_state);

    /* TransferData Upload Block 1 */
    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(10, resp_len); /* 2 header + 8 bytes payload */

    /* Retransmit same block sequence counter 0x01 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);

    /* RequestTransferExit */
    req[0] = SYN_UDS_SID_REQUEST_TRANSFER_EXIT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x77, resp[0]);

    /* 2. ResponseOnEvent (0x86) subfunction 0x04 (reportActivatedEvents) */
    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    req[1] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC6, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp[2]);

    /* 3. RequestFileTransfer (0x38) without registered callback */
    req[0] = SYN_UDS_SID_REQUEST_FILE_TRANSFER;
    req[1] = 0x01;
    req[2] = 'a';
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x78, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);

    /* 4. ReadMemoryByAddress (0x23) without registered memory_cb */
    req[0] = SYN_UDS_SID_READ_MEMORY_BY_ADDRESS;
    req[1] = 0x11;
    req[2] = 0x10;
    req[3] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x63, resp[0]);
    TEST_ASSERT_EQUAL_UINT16(5, resp_len);

    /* 5. DynamicallyDefineDataIdentifier (0x2C) invalid subfunction -> NRC 0x12 */
    req[0] = SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER;
    req[1] = 0x05;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 6. ReadDataByPeriodicIdentifier (0x2A) invalid mode -> NRC 0x31 */
    req[0] = SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER;
    req[1] = 0x05;
    req[2] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 7. WriteMemoryByAddress (0x3D) when security is locked -> NRC 0x33 */
    server.security_state = SYN_UDS_SECURITY_LOCKED;
    req[0] = SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS;
    req[1] = 0x11;
    req[2] = 0x10;
    req[3] = 0x04;
    req[4] = 0x55;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);
}

static bool mock_failing_file_transfer_cb(uint8_t mode, const char *file_path, uint16_t path_len,
                                          uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                          void *ctx)
{
    (void)mode;
    (void)file_path;
    (void)path_len;
    (void)out_buf;
    (void)max_out_len;
    (void)out_len;
    (void)ctx;
    return false;
}

static void test_uds_negative_response_codes(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x010203U, 0x01U, 0x60U));

    uint8_t req[32] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* 1. Small response buffer (NRC 0x14) across ReadDTCInformation (0x19) subfunctions */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x01;
    req[2] = 0x01;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x0A;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x0B;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x08;
    req[2] = 0x60;
    req[3] = 0x01;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 4, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x14;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 2. ReadDataByIdentifier (0x22) small response buffer -> NRC 0x14 */
    TEST_ASSERT_TRUE(syn_uds_register_did(&server, 0xF190, req, 4, false));
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    syn_poke_u16(0xF190, req, 1);
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 3, resp, 3, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 3. ReadMemoryByAddress (0x23) small response buffer -> NRC 0x14 */
    req[0] = SYN_UDS_SID_READ_MEMORY_BY_ADDRESS;
    req[1] = 0x11;
    req[2] = 0x10;
    req[3] = 0x10;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 4, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 4. Incorrect message lengths across services -> NRC 0x13 */
    req[0] = SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_CONTROL_DTC_SETTING;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_ROUTINE_CONTROL;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[0] = SYN_UDS_SID_LINK_CONTROL;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 5. Subfunction / Mode bounds errors -> NRC 0x12 / NRC 0x31 */
    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    req[0] = SYN_UDS_SID_ROUTINE_CONTROL;
    req[1] = 0x99;
    syn_poke_u16(0x0201, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    req[0] = SYN_UDS_SID_LINK_CONTROL;
    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    req[0] = SYN_UDS_SID_REQUEST_FILE_TRANSFER;
    req[1] = 0x99;
    req[2] = 'a';
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 6. RequestFileTransfer (0x38) failing callback -> NRC 0x22 */
    syn_uds_register_file_transfer(&server, mock_failing_file_transfer_cb, NULL);
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
}

static bool mock_failing_auth_cb(uint8_t subfunction, const uint8_t *req_payload, uint16_t req_len,
                                 uint8_t *resp_payload, uint16_t max_resp_len, uint16_t *resp_len,
                                 void *ctx)
{
    (void)subfunction;
    (void)req_payload;
    (void)req_len;
    (void)resp_payload;
    (void)max_resp_len;
    (void)resp_len;
    (void)ctx;
    return false;
}

static bool mock_failing_secured_data_cb(const uint8_t *req_payload, uint16_t req_len,
                                         uint8_t *resp_payload, uint16_t max_resp_len,
                                         uint16_t *resp_len, void *ctx)
{
    (void)req_payload;
    (void)req_len;
    (void)resp_payload;
    (void)max_resp_len;
    (void)resp_len;
    (void)ctx;
    return false;
}

static void test_uds_negative_response_codes_extended(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    uint8_t req[32] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* ReadDataByIdentifier response too long check */
    static uint8_t did_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    syn_uds_register_did(&server, 0x1234, did_data, 10, true);
    uint8_t read_did_req[3] = {SYN_UDS_SID_READ_DATA_BY_IDENTIFIER, 0x12, 0x34};
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, read_did_req, 3, resp, 5, &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 1. AccessTimingParameter (0x83) subfunctions */
    req[0] = SYN_UDS_SID_ACCESS_TIMING_PARAMETER;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 4, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    syn_uds_register_access_timing(&server, mock_failing_timing_cb, NULL);
    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    req[1] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 4, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 2. ReadDTCInformation fallback default subfunction without registered callback */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = 0x07; /* REPORT_NUMBER_BY_SEVERITY_MASK short length */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x08; /* REPORT_BY_SEVERITY_MASK short length */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x09; /* REPORT_SEVERITY_INFO short length */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x7E;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 3. RequestDownload (0x34) when security is locked -> NRC 0x33 */
    server.security_state = SYN_UDS_SECURITY_LOCKED;
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00;
    req[2] = 0x11;
    req[3] = 0x10;
    req[4] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    server.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SID_REQUEST_DOWNLOAD + 0x40, resp[0]);

    /* 4. ReadMemoryByAddress (0x23) failing callback -> NRC 0x22 */
    syn_uds_register_memory_handler(&server, mock_failing_memory_cb, NULL);
    req[0] = SYN_UDS_SID_READ_MEMORY_BY_ADDRESS;
    req[1] = 0x11;
    req[2] = 0x10;
    req[3] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 5. WriteMemoryByAddress (0x3D) failing callback -> NRC 0x22 */
    req[0] = SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS;
    req[1] = 0x11;
    req[2] = 0x10;
    req[3] = 0x04;
    req[4] = 0xAA;
    req[5] = 0xBB;
    req[6] = 0xCC;
    req[7] = 0xDD;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 8, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 6. Authentication (0x29) length error / invalid subfunction / failing callback */
    req[0] = SYN_UDS_SID_AUTHENTICATION;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    syn_uds_register_auth_handler(&server, mock_failing_auth_cb, NULL);
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 7. SecuredDataTransmission (0x84) length error / failing callback */
    req[0] = SYN_UDS_SID_SECURED_DATA_TRANSMISSION;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    syn_uds_register_secured_data(&server, mock_failing_secured_data_cb, NULL);
    req[1] = 0xAA;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
}

static void test_uds_dtc_overflow_and_short_msg_nrcs(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    /* 1. Register max (32) DTCs and verify 33rd registration fails */
    for (uint32_t i = 0; i < 32; i++) {
        TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x100000 + i, 0x09, 0x01));
    }
    TEST_ASSERT_FALSE(syn_uds_register_dtc(&server, 0x100099, 0x09, 0x01));

    uint8_t req[16] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* 2. ReadDTCInformation short request (<2 bytes) */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 3. ReportNumberByStatusMask short request (<3 bytes) */
    req[1] = SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 4. ReportDTCByStatusMask short request (<3 bytes) */
    req[1] = SYN_UDS_DTC_REPORT_BY_STATUS_MASK;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 5. ReportNumberBySeverityMask short request (<4 bytes) */
    req[1] = SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 6. ReportDTCBySeverityMask short request (<4 bytes) */
    req[1] = SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 7. ControlDTCSetting unsupported subfunction */
    req[0] = SYN_UDS_SID_CONTROL_DTC_SETTING;
    req[1] = 0x05;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 8. RequestDownload short request (<3 bytes) */
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 9. TransferData short request (<2 bytes) */
    req[0] = SYN_UDS_SID_TRANSFER_DATA;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 1, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 10. ReadMemoryByAddress short request (<3 bytes) and header mismatch */
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;
    req[0] = SYN_UDS_SID_READ_MEMORY_BY_ADDRESS;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x22; /* 2 bytes addr, 2 bytes size -> total header = 2 + 2 + 2 = 6 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 11. WriteMemoryByAddress short request (<3 bytes) and header mismatch */
    req[0] = SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    req[1] = 0x22; /* header = 6, but payload length = 5 (< 6) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 12. ReadDTCInformation small resp_buf_size < 6 (Line 666, Line 968) */
    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;
    req[1] = SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK;
    req[2] = 0xFF;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    req[1] = SYN_UDS_DTC_REPORT_BY_STATUS_MASK;
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 13. ResponseOnEvent small resp_buf_size < 4 (Line 1110) */
    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    req[1] = 0x00; /* stopResponseOnEvent */
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 2, resp, 3, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 14. RequestDownload small resp_buf_size < 4 (Line 1226) */
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;
    req[0] = SYN_UDS_SID_REQUEST_DOWNLOAD;
    req[1] = 0x00; /* dataFormatIdentifier */
    req[2] = 0x11; /* addrLenFormat: 1 byte addr, 1 byte size */
    req[3] = 0x10; /* address */
    req[4] = 0x10; /* size */
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req, 5, resp, 3, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);
}

static bool mock_dtc_cb_fail_fn(uint8_t sub, const uint8_t *req, uint16_t req_len, uint8_t *resp,
                                uint16_t max_resp_len, uint16_t *out_len, void *user_ctx);
static bool mock_dtc_cb_ok_fn(uint8_t sub, const uint8_t *req, uint16_t req_len, uint8_t *resp,
                              uint16_t max_resp_len, uint16_t *out_len, void *user_ctx);

static void test_uds_read_dtc_by_status_mask(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x012345U, 0x09U, 0xA0U));

    uint8_t req[16] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    req[0] = SYN_UDS_SID_READ_DTC_INFORMATION;

    /* 1. Subfunctions requiring req_len >= 6 (0x04, 0x06, 0x10) */
    uint8_t subs6[] = {0x04, 0x06, 0x10};
    for (size_t i = 0; i < sizeof(subs6); i++) {
        req[1] = subs6[i];
        req[2] = 0x01;
        req[3] = 0x23;
        req[4] = 0x45;
        req[5] = 0x01;
        /* Short request (<6 bytes) -> NRC 0x13 */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

        /* Valid request (6 bytes) with registered DTC -> Positive response (0x59, sub) with
         * statusOfDTC */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(subs6[i], resp[1]);
        TEST_ASSERT_EQUAL_HEX8(0x09, resp[5]); /* statusOfDTC */
        TEST_ASSERT_EQUAL_UINT16(6, resp_len);

        /* Unregistered DTC (0x999999) -> NRC 0x31 RequestOutOfRange */
        req[2] = 0x99;
        req[3] = 0x99;
        req[4] = 0x99;
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
        req[2] = 0x01;
        req[3] = 0x23;
        req[4] = 0x45;
    }

    /* 2. Subfunctions requiring req_len >= 7 (0x18, 0x19) */
    uint8_t subs7[] = {0x18, 0x19};
    for (size_t i = 0; i < sizeof(subs7); i++) {
        req[1] = subs7[i];
        req[2] = 0x01;
        req[3] = 0x23;
        req[4] = 0x45;
        req[5] = 0x01;
        req[6] = 0x01;
        /* Short request (<7 bytes) -> NRC 0x13 */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

        /* Valid request (7 bytes) with registered DTC -> Positive response (0x59, sub) with
         * statusOfDTC */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 7, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(subs7[i], resp[1]);
        TEST_ASSERT_EQUAL_HEX8(0x09, resp[6]); /* statusOfDTC */
        TEST_ASSERT_EQUAL_UINT16(7, resp_len);

        /* Unregistered DTC (0x999999) -> NRC 0x31 RequestOutOfRange */
        req[2] = 0x99;
        req[3] = 0x99;
        req[4] = 0x99;
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 7, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
        req[2] = 0x01;
        req[3] = 0x23;
        req[4] = 0x45;
    }

    /* 3. Subfunction 0x03 (valid at req_len >= 2) */
    req[1] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[1]);

    /* Subfunctions requiring req_len >= 3 (0x05, 0x16) */
    uint8_t subs3[] = {0x05, 0x16};
    for (size_t i = 0; i < sizeof(subs3); i++) {
        req[1] = subs3[i];
        req[2] = 0x01;
        /* Short request (<3 bytes) -> NRC 0x13 */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

        /* Valid request (3 bytes) -> Positive response */
        TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(subs3[i], resp[1]);
        TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    }

    /* 3. Subfunction 0x17 (requiring req_len >= 4) */
    req[1] = 0x17;
    req[2] = 0xFF;
    req[3] = 0x01;
    /* Short request (<4 bytes) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Valid request (4 bytes) -> Positive response (4 bytes) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x17, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(4, resp_len);

    /* 4. Subfunction 0x42 (requiring req_len >= 8) */
    uint8_t req42[8] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x42, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Short request (<8 bytes) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req42, 7, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Valid request (8 bytes) -> Positive response (4 bytes) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req42, 8, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x42, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(4, resp_len);

    /* 5. Subfunction 0x09 (severity info) & 0x55 (WWH-OBD permanent status) with registered DTC */
    uint8_t req09[5] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x09, 0x01, 0x23, 0x45};
    /* Short request (<5 bytes) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req09, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Valid 5-byte request with registered DTC -> 8-byte response */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req09, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x09, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0xA0, resp[2]); /* Severity */
    TEST_ASSERT_EQUAL_UINT16(8, resp_len);

    /* Unregistered DTC lookup for 0x09 -> 2-byte default response */
    uint8_t req09_unreg[5] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x09, 0x99, 0x99, 0x99};
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req09_unreg, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x09, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(2, resp_len);

    /* Subfunction 0x55 */
    uint8_t req55[4] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x55, 0x01, 0x01};
    /* Short request (<4 bytes) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req55, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* Valid 4-byte request -> positive response */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req55, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x55, resp[1]);

    /* Small buffer NRC 0x14 (RESPONSE_TOO_LONG) coverage for 0x09, 0x06, 0x18 */
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req09, 5, resp, 6, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t req06[6] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x06, 0x01, 0x23, 0x45, 0x01};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req06, 6, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t req18[7] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x18, 0x01, 0x23, 0x45, 0x01, 0x01};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&server, req18, 7, resp, 6, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* Coverage for 0x55 buffer overflow (line 932) */
    SYN_UDS_Server server_overflow;
    TEST_ASSERT_TRUE(syn_uds_init(&server_overflow));
    for (uint32_t i = 1; i <= 5; i++) {
        TEST_ASSERT_TRUE(syn_uds_register_dtc(&server_overflow, i, 0x09, 0x01));
    }
    TEST_ASSERT_TRUE(syn_uds_process_request(&server_overflow, req55, 4, resp, 10, &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* dtc_cb delegation for 0x42 and vendor sub-function 0x7E (lines 1257-1265, 1280-1288) */
    server.dtc_cb = mock_dtc_cb_ok_fn;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req42, 8, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x42, resp[1]);

    uint8_t req7e[3] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x7E, 0x01};
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req7e, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x7E, resp[1]);

    server.dtc_cb = mock_dtc_cb_fail_fn;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req42, 8, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req7e, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);
}

static void test_uds_clear_dtc_group_filtering(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    /* Register DTCs across Emissions (E), Powertrain (P), Chassis (C), Body (B), Network (U) */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x000100, 0x09, 0x01)); /* Emissions */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x150000, 0x09, 0x01)); /* Powertrain */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x450000, 0x09, 0x01)); /* Chassis */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x850000, 0x09, 0x01)); /* Body */
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0xC50000, 0x09, 0x01)); /* Network */
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);

    uint8_t req[16] = {0};
    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    req[0] = SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION;

    /* 1. Incorrect request length (!= 4) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 2. Unsupported groupOfDTC -> NRC 0x31 */
    req[1] = 0x99;
    req[2] = 0x99;
    req[3] = 0x99;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* 3. Clear Emissions group (0x000000) */
    req[1] = 0x00;
    req[2] = 0x00;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, server.dtc_table[0].status);
    TEST_ASSERT_NOT_EQUAL(0x50U, server.dtc_table[1].status);

    /* 4. Clear Powertrain group (0x100000) */
    req[1] = 0x10;
    req[2] = 0x00;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, server.dtc_table[1].status);
    TEST_ASSERT_NOT_EQUAL(0x50U, server.dtc_table[2].status);

    /* 4. Clear Chassis group (0x400000) */
    req[1] = 0x40;
    req[2] = 0x00;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, server.dtc_table[2].status);
    TEST_ASSERT_NOT_EQUAL(0x50U, server.dtc_table[3].status);

    /* 5. Clear Body group (0x800000) */
    req[1] = 0x80;
    req[2] = 0x00;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, server.dtc_table[3].status);
    TEST_ASSERT_NOT_EQUAL(0x50U, server.dtc_table[4].status);

    /* 6. Clear Network group (0xC00000) */
    req[1] = 0xC0;
    req[2] = 0x00;
    req[3] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, server.dtc_table[4].status);

    /* 7. Clear All DTCs (0xFFFFFF) */
    req[1] = 0xFF;
    req[2] = 0xFF;
    req[3] = 0xFF;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(5, server.dtc_count);

    /* 6. Clear All DTCs when no DTCs stored -> Positive response 0x54 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54, resp[0]);
}

static void test_uds_dtc_iso14229_bit_operations(void)
{
    SYN_UDS_Server server;
    syn_uds_init(&server);

    /* NULL checks */
    TEST_ASSERT_FALSE(syn_uds_dtc_report_test_result(NULL, 0x010203U, true));
    TEST_ASSERT_FALSE(syn_uds_dtc_start_operation_cycle(NULL));
    TEST_ASSERT_FALSE(syn_uds_dtc_get_status(NULL, 0x010203U, NULL));
    uint8_t status = 0;
    TEST_ASSERT_FALSE(syn_uds_dtc_get_status(&server, 0x010203U, NULL));

    /* Register DTC with initial status */
    uint8_t init_status = SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE |
                          SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&server, 0x010203U, init_status,
                                          SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY));

    /* Check status retrieval */
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_EQUAL_HEX8(init_status, status);

    /* Report test failure */
    TEST_ASSERT_TRUE(syn_uds_dtc_report_test_result(&server, 0x010203U, true));
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_TEST_FAILED) != 0);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE) != 0);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_PENDING_DTC) != 0);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_CONFIRMED_DTC) != 0);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR) != 0);
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE);
    TEST_ASSERT_EQUAL_INT8(127, server.dtc_table[0].fault_cnt);

    /* Report test pass */
    TEST_ASSERT_TRUE(syn_uds_dtc_report_test_result(&server, 0x010203U, false));
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_TEST_FAILED);
    TEST_ASSERT_EQUAL_INT8(-128, server.dtc_table[0].fault_cnt);

    /* Start new operation cycle (cycle 2) - previous cycle failed, pendingDTC retained */
    TEST_ASSERT_TRUE(syn_uds_dtc_start_operation_cycle(&server));
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE) != 0);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_PENDING_DTC) != 0);

    /* Cycle 2: test completes with pass (no failure in this cycle) */
    TEST_ASSERT_TRUE(syn_uds_dtc_report_test_result(&server, 0x010203U, false));
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE);
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_PENDING_DTC) != 0);

    /* Start new operation cycle (cycle 3) - ISO 14229-1: test completed cleanly in cycle 2,
     * pendingDTC clears */
    TEST_ASSERT_TRUE(syn_uds_dtc_start_operation_cycle(&server));
    TEST_ASSERT_TRUE(syn_uds_dtc_get_status(&server, 0x010203U, &status));
    TEST_ASSERT_EQUAL_HEX8(0, status & SYN_UDS_DTC_STATUS_PENDING_DTC);
    TEST_ASSERT_TRUE((status & SYN_UDS_DTC_STATUS_CONFIRMED_DTC) != 0); /* confirmedDTC remains */

    /* Non-existent DTC search */
    TEST_ASSERT_FALSE(syn_uds_dtc_report_test_result(&server, 0x999999U, true));
    TEST_ASSERT_FALSE(syn_uds_dtc_get_status(&server, 0x999999U, &status));
}

static bool mock_dtc_cb_fail_fn(uint8_t sub, const uint8_t *req, uint16_t req_len, uint8_t *resp,
                                uint16_t max_resp_len, uint16_t *out_len, void *user_ctx)
{
    (void)sub;
    (void)req;
    (void)req_len;
    (void)resp;
    (void)max_resp_len;
    (void)out_len;
    (void)user_ctx;
    return false;
}

static bool mock_dtc_cb_ok_fn(uint8_t sub, const uint8_t *req, uint16_t req_len, uint8_t *resp,
                              uint16_t max_resp_len, uint16_t *out_len, void *user_ctx)
{
    (void)sub;
    (void)req;
    (void)req_len;
    (void)resp;
    (void)max_resp_len;
    (void)out_len;
    (void)user_ctx;
    *out_len = 2;
    resp[0] = 0x11;
    resp[1] = 22;
    return true;
}

static bool mock_mem_cb_fail_fn(bool write, uint32_t addr, uint32_t len, uint8_t *buf,
                                void *user_ctx)
{
    (void)write;
    (void)addr;
    (void)len;
    (void)buf;
    (void)user_ctx;
    return false;
}

static void test_uds_security_and_routine_error_handling(void)
{
    SYN_UDS_Server server;
    syn_uds_init(&server);
    uint8_t resp[256];
    uint16_t resp_len = 0;

    /* 1. Clear DTC group filtering (lines 604, 636-637, 659, 661) */
    syn_uds_register_dtc(&server, 0x812345U, SYN_UDS_DTC_STATUS_CONFIRMED_DTC,
                         SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY); /* Body 0x800000..0x8EFFFF */
    syn_uds_register_dtc(&server, 0x412345U, SYN_UDS_DTC_STATUS_CONFIRMED_DTC,
                         SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY); /* Chassis 0x400000..0x4EFFFF */
    syn_uds_register_dtc(&server, 0xC12345U, SYN_UDS_DTC_STATUS_CONFIRMED_DTC,
                         SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY); /* Network 0xC00000..0xFEFFFF */

    uint8_t req_clear_body[4] = {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 0x81, 0x23, 0x45};
    syn_uds_process_request(&server, req_clear_body, 4, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    uint8_t req_clear_chassis[4] = {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 0x41, 0x23, 0x45};
    syn_uds_process_request(&server, req_clear_chassis, 4, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    uint8_t req_clear_net[4] = {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 0xC1, 0x23, 0x45};
    syn_uds_process_request(&server, req_clear_net, 4, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    uint8_t req_clear_all[4] = {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 0xFF, 0xFF, 0xFF};
    syn_uds_process_request(&server, req_clear_all, 4, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    /* 2. Read DTC subfunctions with dtc_cb (lines 894-905, 925, 948-956, 974-982, 996-1004) */
    server.dtc_cb = mock_dtc_cb_ok_fn;
    uint8_t subs[] = {0x05, 0x06, 0x09, 0x17, 0x18};
    for (size_t i = 0; i < sizeof(subs); i++) {
        uint8_t req_dtc[8] = {
            SYN_UDS_SID_READ_DTC_INFORMATION, subs[i], 0x01, 0x02, 0x03, 0xFF, 0x01, 0x00};
        uint16_t req_len_sub = (subs[i] == 0x18) ? 7 : 6;
        syn_uds_process_request(&server, req_dtc, req_len_sub, resp, sizeof(resp), &resp_len,
                                SYN_UDS_ADDR_PHYSICAL);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SID_READ_DTC_INFORMATION + 0x40, resp[0]);
    }

    server.dtc_cb = mock_dtc_cb_fail_fn;
    uint8_t subs_nrc[] = {0x05, 0x06, 0x09, 0x17, 0x18};
    for (size_t i = 0; i < sizeof(subs_nrc); i++) {
        uint8_t req_dtc[8] = {
            SYN_UDS_SID_READ_DTC_INFORMATION, subs_nrc[i], 0x01, 0x02, 0x03, 0xFF, 0x01, 0x00};
        uint16_t req_len_sub = (subs_nrc[i] == 0x18) ? 7 : 6;
        syn_uds_process_request(&server, req_dtc, req_len_sub, resp, sizeof(resp), &resp_len,
                                SYN_UDS_ADDR_PHYSICAL);
        TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    }
    uint8_t subs_pos[] = {0x0B, 0x0C, 0x0D};
    for (size_t i = 0; i < sizeof(subs_pos); i++) {
        uint8_t req_dtc[2] = {SYN_UDS_SID_READ_DTC_INFORMATION, subs_pos[i]};
        syn_uds_process_request(&server, req_dtc, 2, resp, sizeof(resp), &resp_len,
                                SYN_UDS_ADDR_PHYSICAL);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SID_READ_DTC_INFORMATION + 0x40, resp[0]);
    }
    server.dtc_cb = NULL;

    /* 3. ReadDataByIdentifier max_resp_len < 3 (line 209) */
    uint8_t req_rdbi[3] = {SYN_UDS_SID_READ_DATA_BY_IDENTIFIER, 0xF1, 0x90};
    TEST_ASSERT_FALSE(
        syn_uds_process_request(&server, req_rdbi, 3, resp, 2, &resp_len, SYN_UDS_ADDR_PHYSICAL));

    /* 4. WriteDataByIdentifier max_resp_len < 3 (line 209) */
    uint8_t req_wdbi[4] = {SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER, 0xF1, 0x90, 0x01};
    TEST_ASSERT_FALSE(
        syn_uds_process_request(&server, req_wdbi, 4, resp, 2, &resp_len, SYN_UDS_ADDR_PHYSICAL));

    /* 5. ControlDTCSetting req_len < 2 (line 1019) */
    uint8_t req_ctrl_dtc[1] = {SYN_UDS_SID_CONTROL_DTC_SETTING};
    syn_uds_process_request(&server, req_ctrl_dtc, 1, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);

    /* 4. Transfer data overflow & read mem_cb fail (lines 1135, 1161, 1165-1168) */
    server.transfer_state = SYN_UDS_TRANSFER_DOWNLOAD;
    server.transfer_size = 5;
    server.transfer_bytes_processed = 4;
    uint8_t req_td_overflow[6] = {SYN_UDS_SID_TRANSFER_DATA, 0x01, 0xAA, 0xBB, 0xCC, 0xDD};
    syn_uds_process_request(&server, req_td_overflow, 6, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);

    server.transfer_state = SYN_UDS_TRANSFER_UPLOAD;
    server.memory_cb = mock_mem_cb_fail_fn;
    uint8_t req_td_read_fail[3] = {SYN_UDS_SID_TRANSFER_DATA, 0x01, 0x02};
    syn_uds_process_request(&server, req_td_read_fail, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);

    /* 5. Read Scaling Data By Identifier bounds (lines 1237, 1242) */
    uint8_t req_scaling_short[2] = {SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER, 0x01};
    syn_uds_process_request(&server, req_scaling_short, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);

    uint8_t req_scaling_ok[3] = {SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER, 0x01, 0x02};
    syn_uds_process_request(&server, req_scaling_ok, 3, resp, 3, &resp_len, SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);

    /* 6. Request Download incorrect length (line 1385) */
    uint8_t req_dl_short[4] = {SYN_UDS_SID_REQUEST_DOWNLOAD, 0x00, 0x11, 0x11};
    syn_uds_process_request(&server, req_dl_short, 4, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
}

static bool dummy_sec_cb_fail(const uint8_t *in, uint16_t in_len, uint8_t *out, uint16_t max_out,
                              uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)out;
    (void)max_out;
    (void)out_len;
    (void)ctx;
    return false;
}

static bool dummy_dtc_cb_fail(uint8_t sub, const uint8_t *req, uint16_t req_len, uint8_t *resp,
                              uint16_t max_resp, uint16_t *out_len, void *ctx)
{
    (void)sub;
    (void)req;
    (void)req_len;
    (void)resp;
    (void)max_resp;
    (void)out_len;
    (void)ctx;
    return false;
}

static bool dummy_mem_cb_pass(bool is_write, uint32_t addr, uint32_t len, uint8_t *data, void *ctx)
{
    (void)is_write;
    (void)addr;
    (void)len;
    (void)data;
    (void)ctx;
    return true;
}

static void test_uds_dtc_and_transfer_boundary_conditions(void)
{
    SYN_UDS_Server server;
    syn_uds_init(&server);
    uint8_t resp[256];
    uint16_t resp_len = 0;

    /* Extended session */
    uint8_t sess[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_EXTENDED};
    syn_uds_process_request(&server, sess, 2, resp, sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL);
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;

    /* 1. SecuredDataTransmission callback returning false (L600) */
    server.secured_data_cb = dummy_sec_cb_fail;
    uint8_t sec_req[3] = {SYN_UDS_SID_SECURED_DATA_TRANSMISSION, 0x01, 0x02};
    syn_uds_process_request(&server, sec_req, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 2. ReadDTCInformation subfunction 0x03 with callback returning false (L921) */
    server.dtc_cb = dummy_dtc_cb_fail;
    uint8_t dtc_req[3] = {SYN_UDS_SID_READ_DTC_INFORMATION,
                          SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION, 0x01};
    syn_uds_process_request(&server, dtc_req, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* 3. ControlDTCSetting subfunction > 0x07 (L1042) */
    uint8_t dtc_set_req[2] = {SYN_UDS_SID_CONTROL_DTC_SETTING, 0x08};
    syn_uds_process_request(&server, dtc_set_req, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 4. WriteMemoryByAddress multi-byte size_len (L1381) */
    server.memory_cb = dummy_mem_cb_pass;
    uint8_t write_addr_req[8] = {
        SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS, 0x22, 0x10, 0x00, 0x00, 0x02, 0xAA, 0xBB};
    syn_uds_process_request(&server, write_addr_req, 8, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS + 0x40, resp[0]);

    /* 5. ReadDTCInformation small resp_buf_size < 6 -> NRC 0x14 Response Too Long (L905) */
    server.dtc_cb = NULL;
    uint8_t dtc_req2[3] = {SYN_UDS_SID_READ_DTC_INFORMATION, 0x01, 0xFF};
    syn_uds_process_request(&server, dtc_req2, 3, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 6. ResponseOnEvent subfunction > 0x07 -> NRC 0x12 Subfunction Not Supported (L1042) */
    uint8_t roe_req[2] = {SYN_UDS_SID_RESPONSE_ON_EVENT, 0x08};
    syn_uds_process_request(&server, roe_req, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_HEX8(0x7F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    /* 7. TransferData with remaining < chunk_len (L1157) */
    server.transfer_state = 1; /* Transfer in progress */
    server.transfer_size = 10;
    server.transfer_bytes_processed = 5;
    uint8_t xfer_req[7] = {SYN_UDS_SID_TRANSFER_DATA, 0x01, 'H', 'e', 'l', 'l', 'o'};
    syn_uds_process_request(&server, xfer_req, 7, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
}

static uint8_t g_test_reset_cb_type = 0;
static void *g_test_reset_cb_ctx = NULL;

static void test_reset_callback_func(uint8_t reset_type, void *ctx)
{
    g_test_reset_cb_type = reset_type;
    g_test_reset_cb_ctx = ctx;
}

static void test_uds_deferred_reset_callback(void)
{
    syn_uds_init(&g_uds);
    g_test_reset_cb_type = 0;
    g_test_reset_cb_ctx = NULL;

    /* Register reset handler and set custom wait timer (30ms) */
    syn_uds_set_reset_handler(&g_uds, test_reset_callback_func, (void *)0x1234);
    syn_uds_set_reset_wait_ms(&g_uds, 30);

    /* Send ECUReset request (hardReset = 0x01) */
    uint8_t req[2] = {SYN_UDS_SID_ECU_RESET, 0x01};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x51, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, syn_uds_get_pending_reset(&g_uds));
    TEST_ASSERT_EQUAL_HEX8(0, g_test_reset_cb_type);

    /* Advance by 20ms -> Callback not yet fired */
    syn_uds_tick(&g_uds, 20);
    TEST_ASSERT_EQUAL_HEX8(0x01, syn_uds_get_pending_reset(&g_uds));
    TEST_ASSERT_EQUAL_HEX8(0, g_test_reset_cb_type);

    /* Advance by another 10ms -> Total 30ms, callback fires, pending reset cleared */
    syn_uds_tick(&g_uds, 10);
    TEST_ASSERT_EQUAL_HEX8(0, syn_uds_get_pending_reset(&g_uds));
    TEST_ASSERT_EQUAL_HEX8(0x01, g_test_reset_cb_type);
    TEST_ASSERT_EQUAL_PTR((void *)0x1234, g_test_reset_cb_ctx);

    /* Test null server handling in setter functions */
    syn_uds_set_reset_handler(NULL, NULL, NULL);
    syn_uds_set_reset_wait_ms(NULL, 100);
}
static void test_uds_response_length_exceeded(void)
{
    SYN_UDS_Server server;
    syn_uds_init(&server);
    uint8_t resp[16];
    uint16_t resp_len = 0;

    /* SIDs with short buffer (resp_max_len = 2) */
    uint8_t sids[] = {0x10, 0x11, 0x22, 0x27, 0x28, 0x2E, 0x31,
                      0x34, 0x35, 0x36, 0x37, 0x3E, 0x85, 0x87};
    for (size_t i = 0; i < sizeof(sids); i++) {
        uint8_t req[8] = {sids[i], 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        syn_uds_process_request(&server, req, sizeof(req), resp, 2, &resp_len,
                                SYN_UDS_ADDR_PHYSICAL);
    }

    /* SID 0x19 short requests */
    uint8_t req_19_1[1] = {0x19};
    syn_uds_process_request(&server, req_19_1, 1, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_19_2[2] = {0x19, 0x01};
    syn_uds_process_request(&server, req_19_2, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_19_3[2] = {0x19, 0x02};
    syn_uds_process_request(&server, req_19_3, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_19_4[3] = {0x19, 0x07, 0x00};
    syn_uds_process_request(&server, req_19_4, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_19_5[3] = {0x19, 0x08, 0x00};
    syn_uds_process_request(&server, req_19_5, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_19_6[5] = {0x19, 0x04, 0x01, 0x02, 0x03};
    syn_uds_process_request(&server, req_19_6, 5, resp, 5, &resp_len, SYN_UDS_ADDR_PHYSICAL);

    /* SID 0x3D truncated data test */
    uint8_t req_3d[6] = {0x3D, 0x11, 0x10,
                         0x10, 0x00, 0x00}; /* addr_len=1, size_len=1, size=16, req_len=6 < 4+16 */
    syn_uds_process_request(&server, req_3d, sizeof(req_3d), resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    /* SID 0x85 invalid subfunction */
    uint8_t req_85[2] = {0x85, 0x99};
    syn_uds_process_request(&server, req_85, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    /* SID 0x34 short request */
    uint8_t req_34[2] = {0x34, 0x00};
    syn_uds_process_request(&server, req_34, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    /* SID 0x23, 0x24, 0x3D short requests */
    server.security_state = SYN_UDS_SECURITY_UNLOCKED;
    uint8_t req_23_short[2] = {0x23, 0x11};
    syn_uds_process_request(&server, req_23_short, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_23_short2[3] = {0x23, 0x22, 0x00}; /* addr_len=2, size_len=2 -> req_len=3 < 2+4 */
    syn_uds_process_request(&server, req_23_short2, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);

    uint8_t req_24_short[2] = {0x24, 0x00};
    syn_uds_process_request(&server, req_24_short, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_24_ok[3] = {0x24, 0x12, 0x34};
    syn_uds_process_request(&server, req_24_ok, 3, resp, 2, &resp_len,
                            SYN_UDS_ADDR_PHYSICAL); /* resp_max_len = 2 < 4 */

    uint8_t req_3d_short[2] = {0x3D, 0x11};
    syn_uds_process_request(&server, req_3d_short, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    uint8_t req_3d_short2[3] = {0x3D, 0x22, 0x00}; /* addr_len=2, size_len=2 -> req_len=3 < 2+4 */
    syn_uds_process_request(&server, req_3d_short2, 3, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    ;

    /* Register DTC handler NULL check */
    TEST_ASSERT_FALSE(syn_uds_register_dtc_handler(NULL, NULL, NULL));

    /* SID 0x27 subfunction not supported (line 482) */
    uint8_t req_27_unsupp[2] = {0x27, 0xFF};
    syn_uds_process_request(&server, req_27_unsupp, 2, resp, sizeof(resp), &resp_len,
                            SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);
}

static void pump_isotp_can_frames(SYN_ISOTP_Link *a, SYN_ISOTP_Link *b)
{
    SYN_CAN_Frame f;
    while (syn_isotp_get_tx_frame(a, &f)) {
        syn_isotp_process_rx_frame(b, &f);
    }
    while (syn_isotp_get_tx_frame(b, &f)) {
        syn_isotp_process_rx_frame(a, &f);
    }
}

static void test_uds_isotp_full_stack_loopback(void)
{
    SYN_ISOTP_Link client_tp, server_tp;
    uint8_t c_rx[256], c_tx[256], s_rx[256], s_tx[256];
    syn_isotp_init(&client_tp, 0x7E8, 0x7E0, c_rx, sizeof(c_rx), c_tx, sizeof(c_tx));
    syn_isotp_init(&server_tp, 0x7E0, 0x7E8, s_rx, sizeof(s_rx), s_tx, sizeof(s_tx));

    SYN_UDS_Server server;
    syn_uds_init(&server);
    const uint8_t aes_key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    const uint8_t seed_16[16] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
                                 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x01};
    syn_uds_enable_aes128_security(&server, aes_key);
    syn_uds_set_security_seed_bytes(&server, seed_16);
    syn_uds_tick(&server, 10000);

    /* 1. Client sends SecurityAccess Request Seed (0x27 0x01) over ISO-TP */
    uint8_t req[2] = {SYN_UDS_SID_SECURITY_ACCESS, 0x01};
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&client_tp, req, sizeof(req)));
    pump_isotp_can_frames(&client_tp, &server_tp);

    /* Server receives full request from ISO-TP */
    uint8_t req_in[256];
    ssize_t req_len = syn_isotp_receive(&server_tp, req_in, sizeof(req_in));
    TEST_ASSERT_EQUAL(2, req_len);

    /* Server processes request and transmits multi-frame seed response over ISO-TP */
    uint8_t resp_buf[256];
    uint16_t resp_len = 0;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req_in, (uint16_t)req_len, resp_buf,
                                             sizeof(resp_buf), &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(18, resp_len); /* 18 bytes require multi-frame (FF + CF1 + CF2) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_isotp_send(&server_tp, resp_buf, resp_len));

    /* Pump CAN frames between server and client (FF -> FC -> CF1 -> CF2) */
    for (int i = 0; i < 5; i++) {
        pump_isotp_can_frames(&server_tp, &client_tp);
        pump_isotp_can_frames(&client_tp, &server_tp);
    }

    /* Client receives multi-frame ISO-TP response */
    uint8_t client_rx_buf[256];
    ssize_t client_rx_len = syn_isotp_receive(&client_tp, client_rx_buf, sizeof(client_rx_buf));
    TEST_ASSERT_EQUAL(18, client_rx_len);
    TEST_ASSERT_EQUAL_HEX8(0x67, client_rx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, client_rx_buf[1]);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &client_rx_buf[2], 16);
}

static void test_uds_session_mask_filtering(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    uint8_t dummy_data[4] = {0x11, 0x22, 0x33, 0x44};
    /* Register DID 0xF190 allowed only in EXTENDED session */
    uint8_t ext_mask = SYN_UDS_SESSION_MASK_EXTENDED;
    bool ok = syn_uds_register_did_ext(&server, 0xF190, dummy_data, 4, true, ext_mask,
                                       SYN_UDS_SECURITY_MASK_ALL);
    TEST_ASSERT_TRUE(ok);

    uint8_t req[16] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* 1. Read DID 0xF190 in DEFAULT session -> NRC 0x7E */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp[2]);

    /* 2. Write DID 0xF190 in DEFAULT session -> NRC 0x7E */
    req[0] = SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    req[3] = 0xAA;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp[2]);

    /* 3. Switch to EXTENDED session */
    req[0] = SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    req[1] = SYN_UDS_SESSION_EXTENDED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);

    /* 4. Read DID 0xF190 in EXTENDED session -> Success (0x62) */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x62, resp[0]);

    /* 5. Switch to SAFETY_SYSTEM session */
    req[0] = SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    req[1] = SYN_UDS_SESSION_SAFETY_SYSTEM;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x50, resp[0]);

    /* Read DID 0xF190 in SAFETY_SYSTEM session -> NRC 0x7E */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0xF1;
    req[2] = 0x90;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp[2]);
}

static void test_uds_security_mask_filtering(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    uint8_t secret_data[2] = {0xAA, 0xBB};
    /* Register DID 0x0300 requiring Security Level 1 */
    bool ok = syn_uds_register_did_ext(&server, 0x0300, secret_data, 2, true,
                                       SYN_UDS_SESSION_MASK_ALL, SYN_UDS_SECURITY_MASK_LEVEL_1);
    TEST_ASSERT_TRUE(ok);

    uint8_t req[16] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* Read DID 0x0300 when locked (level 0) -> NRC 0x33 SecurityAccessDenied */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0x03;
    req[2] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* Unlock Level 1 */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    uint32_t key = server.current_seed ^ 0xA5A5A5A5U;
    req[1] = 0x02;
    syn_poke_u32(key, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, server.security_state);
    TEST_ASSERT_EQUAL_UINT8(1, syn_uds_get_security_level(&server));

    /* Read DID 0x0300 now unlocked at Level 1 -> Success 0x62 */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    req[1] = 0x03;
    req[2] = 0x00;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x62, resp[0]);

    /* Test SecurityAccess subfunction mismatch (line 467) */
    server.security_state = SYN_UDS_SECURITY_LOCKED;
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01; /* Request Seed Level 1 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    req[1] = 0x04; /* Mismatched Send Key Level 2 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 6, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp[2]);

    /* Test SecurityAccess short message length for Send Key (line 501) */
    req[1] = 0x01; /* Request Seed Level 1 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    req[1] = 0x02; /* Send Key Level 1 but short len = 5 (< 6) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
}

void test_uds_security_access_repeated_unlock(void)
{
    SYN_UDS_Server server;
    TEST_ASSERT_TRUE(syn_uds_init(&server));

    uint8_t aes_key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                           0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t seed_16[16] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
                           0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    syn_uds_enable_aes128_security(&server, aes_key);
    syn_uds_set_security_seed_bytes(&server, seed_16);

    uint8_t req[32] = {0};
    uint8_t resp[32] = {0};
    uint16_t resp_len = 0;

    /* Pass 1: Request Seed (27 01) */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(18, resp_len);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &resp[2], 16);

    /* Compute expected key */
    SYN_AES_Context aes_ctx;
    syn_aes_init(&aes_ctx, aes_key, 16U);
    uint8_t expected_key[16];
    syn_aes_encrypt_block(&aes_ctx, seed_16, expected_key);

    /* Pass 1: Send Key (27 02) */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x02;
    memcpy(&req[2], expected_key, 16);
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);

    /* Pass 2: Repeat Request Seed (27 01) while UNLOCKED -> returns seed_16 */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_UINT16(18, resp_len);
    TEST_ASSERT_EQUAL_MEMORY(seed_16, &resp[2], 16);

    /* Pass 2: Send Key (27 02) -> Success */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x02;
    memcpy(&req[2], expected_key, 16);
    TEST_ASSERT_TRUE(syn_uds_process_request(&server, req, 18, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x67, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp[1]);
}

static void test_uds_addressing_modes(void)
{
    syn_uds_init(&g_uds);
    uint8_t resp[256];
    uint16_t resp_len = 0;

    /* 1. Physical-only SIDs silences response when requested functionally */
    static const uint8_t req_27[2] = {0x27, 0x01};
    static const uint8_t req_2E[3] = {0x2E, 0xF1, 0x90};
    static const uint8_t req_34[3] = {0x34, 0x00, 0x11};
    static const uint8_t req_35[3] = {0x35, 0x00, 0x11};
    static const uint8_t req_36[2] = {0x36, 0x01};
    static const uint8_t req_37[1] = {0x37};
    static const uint8_t req_38[3] = {0x38, 0x01, 0x00};
    static const uint8_t req_3D[3] = {0x3D, 0x11, 0x00};

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_27, sizeof(req_27), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_2E, sizeof(req_2E), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_34, sizeof(req_34), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_35, sizeof(req_35), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_36, sizeof(req_36), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_37, sizeof(req_37), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_38, sizeof(req_38), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_3D, sizeof(req_3D), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    /* Verify syn_uds_is_sid_functional_supported queries for ISO 14229-1 SIDs */
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x10));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x11));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x14));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x19));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x22));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x28));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x3E));
    TEST_ASSERT_TRUE(syn_uds_is_sid_functional_supported(0x85));

    TEST_ASSERT_FALSE(syn_uds_is_sid_functional_supported(0x23));
    TEST_ASSERT_FALSE(syn_uds_is_sid_functional_supported(0x27));
    TEST_ASSERT_FALSE(syn_uds_is_sid_functional_supported(0x2E));
    TEST_ASSERT_FALSE(syn_uds_is_sid_functional_supported(0x34));

    /* Physical addressing on physical-only SID produces response (e.g. NRC or positive) */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_34, sizeof(req_34), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    /* 2. Functional-supported SIDs respond normally */
    static const uint8_t req_3E[2] = {0x3E, 0x00};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_3E, sizeof(req_3E), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(2, resp_len);
    TEST_ASSERT_EQUAL_UINT8(0x7E, resp[0]);

    /* 3. Functional NRC suppression (0x11, 0x12, 0x7E) */
    /* Unsupported SID 0x99 -> NRC 0x11 */
    static const uint8_t req_unsupp_sid[2] = {0x99, 0x00};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_unsupp_sid, sizeof(req_unsupp_sid), resp,
                                             sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_SERVICE_NOT_SUPPORTED, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_unsupp_sid, sizeof(req_unsupp_sid), resp,
                                             sizeof(resp), &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    /* Unsupported sub-function on 0x3E -> NRC 0x12 */
    static const uint8_t req_unsupp_sub[2] = {0x3E, 0x55};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_unsupp_sub, sizeof(req_unsupp_sub), resp,
                                             sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_unsupp_sub, sizeof(req_unsupp_sub), resp,
                                             sizeof(resp), &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);
}

static void test_uds_custom_service_policies(void)
{
    syn_uds_init(&g_uds);
    uint8_t resp[256];
    uint16_t resp_len = 0;

    /* Null server checks */
    TEST_ASSERT_FALSE(syn_uds_set_service_session_mask(NULL, 0x22, SYN_UDS_SESSION_MASK_EXTENDED));
    TEST_ASSERT_FALSE(syn_uds_set_service_security_mask(NULL, 0x22, SYN_UDS_SECURITY_MASK_LEVEL_1));

    /* Restrict ReadDataByIdentifier (0x22) to EXTENDED session */
    TEST_ASSERT_TRUE(syn_uds_set_service_session_mask(&g_uds, 0x22, SYN_UDS_SESSION_MASK_EXTENDED));

    static const uint8_t req_22[3] = {0x22, 0xF1, 0x90};
    /* In DEFAULT session, 0x22 returns NRC 0x7F */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_22, sizeof(req_22), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp[2]);

    /* Switch to EXTENDED session */
    g_uds.session = SYN_UDS_SESSION_EXTENDED;

    /* Configure SYN_UDS_SECURITY_MASK_NONE for 0x22 while locked */
    g_uds.security_state = SYN_UDS_SECURITY_LOCKED;
    g_uds.security_level = 0;
    TEST_ASSERT_TRUE(syn_uds_set_service_security_mask(&g_uds, 0x22, SYN_UDS_SECURITY_MASK_NONE));

    /* Locked security with SYN_UDS_SECURITY_MASK_NONE allows access (returns NRC 0x31 for unknown
     * DID 0xF190, NOT NRC 0x33) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_22, sizeof(req_22), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* Unlocking Level 1 while SYN_UDS_SECURITY_MASK_NONE is configured preserves Level 0 access */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    g_uds.security_level = 1;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_22, sizeof(req_22), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);
    g_uds.security_state = SYN_UDS_SECURITY_LOCKED;
    g_uds.security_level = 0;

    /* Require Security Level 1 for 0x22 */
    TEST_ASSERT_TRUE(
        syn_uds_set_service_security_mask(&g_uds, 0x22, SYN_UDS_SECURITY_MASK_LEVEL_1));

    /* Locked security returns NRC 0x33 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_22, sizeof(req_22), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* Unlock security level 1 */
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;
    g_uds.security_level = 1;

    /* Now request succeeds (returns NRC 0x31 for unknown DID 0xF190) */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_22, sizeof(req_22), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(3, resp_len);
    TEST_ASSERT_EQUAL_UINT8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* Fill capacity of overrides array (SYN_UDS_MAX_SERVICE_OVERRIDES slots = 8) - 0x22 is 1 slot,
     * so add 7 more */
    for (uint8_t sid = 0x40; sid < 0x47; sid++) {
        TEST_ASSERT_TRUE(
            syn_uds_set_service_session_mask(&g_uds, sid, SYN_UDS_SESSION_MASK_EXTENDED));
        TEST_ASSERT_TRUE(
            syn_uds_set_service_security_mask(&g_uds, sid, SYN_UDS_SECURITY_MASK_LEVEL_1));
    }
    /* 9th override fails */
    TEST_ASSERT_FALSE(
        syn_uds_set_service_session_mask(&g_uds, 0x50, SYN_UDS_SESSION_MASK_EXTENDED));
    TEST_ASSERT_FALSE(
        syn_uds_set_service_security_mask(&g_uds, 0x50, SYN_UDS_SECURITY_MASK_LEVEL_1));
}

static bool test_routine_cb(uint8_t sub, uint16_t id, const uint8_t *in, uint16_t in_len,
                            uint8_t *out, uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)max_out;
    (void)ctx;
    if (sub == 0x01 && id == 0x0201) {
        out[0] = 0xAA;
        *out_len = 1;
        return true;
    }
    return false;
}

static bool test_io_cb(uint16_t did, uint8_t opt, const uint8_t *in, uint16_t in_len, uint8_t *out,
                       uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)max_out;
    (void)ctx;
    if (did == 0x9B00 && opt == 0x03) {
        out[0] = 0x55;
        *out_len = 1;
        return true;
    }
    return false;
}

static bool test_link_cb(uint8_t sub, const uint8_t *in, uint16_t in_len, uint8_t *out,
                         uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)out;
    (void)max_out;
    (void)ctx;
    if (sub == 0x01) {
        *out_len = 0;
        return true;
    }
    return false;
}

static bool test_roe_cb(uint8_t sub, const uint8_t *in, uint16_t in_len, uint8_t *out,
                        uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)ctx;
    if (sub == 0x01 && max_out >= 2) {
        out[0] = 0x01;
        out[1] = 0x00;
        *out_len = 2;
        return true;
    }
    return false;
}

static bool test_scaling_cb(uint16_t did, uint8_t *out, uint16_t max_out, uint16_t *out_len,
                            void *ctx)
{
    (void)ctx;
    if (did == 0xF190 && max_out >= 2) {
        out[0] = 0x01;
        out[1] = 0x11;
        *out_len = 2;
        return true;
    }
    return false;
}

static bool test_periodic_cb(uint8_t mode, const uint8_t *in, uint16_t in_len, uint8_t *out,
                             uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)out;
    (void)max_out;
    (void)ctx;
    if (mode == 0x01) {
        *out_len = 0;
        return true;
    }
    return false;
}

static bool test_dynamic_did_cb(uint8_t sub, uint16_t dyn_did, const uint8_t *in, uint16_t in_len,
                                uint8_t *out, uint16_t max_out, uint16_t *out_len, void *ctx)
{
    (void)in;
    (void)in_len;
    (void)out;
    (void)max_out;
    (void)ctx;
    if (sub == 0x01 && dyn_did == 0xF200) {
        *out_len = 0;
        return true;
    }
    return false;
}

static void test_uds_iso14229_service_callbacks(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16] = {0};
    uint8_t resp[16] = {0};
    uint16_t resp_len = 0;

    /* Test null registration */
    TEST_ASSERT_FALSE(syn_uds_register_routine_control(NULL, test_routine_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_io_control(NULL, test_io_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_link_control(NULL, test_link_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_roe_handler(NULL, test_roe_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_scaling_data_handler(NULL, test_scaling_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_periodic_data_handler(NULL, test_periodic_cb, NULL));
    TEST_ASSERT_FALSE(syn_uds_register_dynamic_did_handler(NULL, test_dynamic_did_cb, NULL));

    /* Register all callbacks */
    TEST_ASSERT_TRUE(syn_uds_register_routine_control(&g_uds, test_routine_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_io_control(&g_uds, test_io_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_link_control(&g_uds, test_link_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_roe_handler(&g_uds, test_roe_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_scaling_data_handler(&g_uds, test_scaling_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_periodic_data_handler(&g_uds, test_periodic_cb, NULL));
    TEST_ASSERT_TRUE(syn_uds_register_dynamic_did_handler(&g_uds, test_dynamic_did_cb, NULL));

    /* Service 0x31 RoutineControl dispatch success & failure */
    req[0] = SYN_UDS_SID_ROUTINE_CONTROL;
    req[1] = 0x01;
    syn_poke_u16(0x0201, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x71, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp[1]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, resp[4]);

    syn_poke_u16(0x9999, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Test 0x31 RoutineControl positive response suppression (bit 7 set) */
    req[0] = SYN_UDS_SID_ROUTINE_CONTROL;
    req[1] = 0x81; /* 0x01 | 0x80 suppressPosRspMsgIndicationBit */
    syn_poke_u16(0x0201, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT16(0, resp_len);

    /* Service 0x2F IOControl dispatch success & failure */
    req[0] = SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER;
    syn_poke_u16(0x9B00, req, 1);
    req[3] = 0x03;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6F, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, resp[3]);
    TEST_ASSERT_EQUAL_HEX8(0x55, resp[4]);

    syn_poke_u16(0x1111, req, 1);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp[2]);

    /* Test 0x2F IOControl invalid control_opt (> 0x03) */
    req[3] = 0x04;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* Test 0x2F IOControl unregistered DID without callback */
    SYN_UDS_Server fresh_uds;
    syn_uds_init(&fresh_uds);
    req[3] = 0x03;
    syn_poke_u16(0x9999, req, 1);
    TEST_ASSERT_TRUE(syn_uds_process_request(&fresh_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp[2]);

    /* Service 0x87 LinkControl dispatch success & failure */
    req[0] = SYN_UDS_SID_LINK_CONTROL;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC7, resp[0]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Service 0x86 ResponseOnEvent dispatch success & failure */
    req[0] = SYN_UDS_SID_RESPONSE_ON_EVENT;
    req[1] = 0x01;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC6, resp[0]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 2, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Service 0x24 ReadScalingData dispatch success & failure */
    req[0] = SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER;
    syn_poke_u16(0xF190, req, 1);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x64, resp[0]);

    syn_poke_u16(0x4444, req, 1);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Service 0x2A ReadDataByPeriodic dispatch success & failure */
    req[0] = SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER;
    req[1] = 0x01;
    req[2] = 0xE0;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6A, resp[0]);

    req[1] = 0x02;
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 3, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);

    /* Service 0x2C DynamicallyDefineDID dispatch success & failure */
    req[0] = SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER;
    req[1] = 0x01;
    syn_poke_u16(0xF200, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6C, resp[0]);

    syn_poke_u16(0x9999, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
}

static void test_uds_multi_did_read(void)
{
    syn_uds_init(&g_uds);
    uint8_t req[16];
    uint8_t resp[64];
    uint16_t resp_len = 0;

    /* Register DID 0x1000 (2 bytes) and DID 0x1001 (4 bytes) */
    static uint8_t did1_data[2] = {0xAA, 0xBB};
    static uint8_t did2_data[4] = {0x11, 0x22, 0x33, 0x44};

    syn_uds_register_did(&g_uds, 0x1000, did1_data, 2, true);
    syn_uds_register_did(&g_uds, 0x1001, did2_data, 4, true);

    /* 1. Request both DIDs in a single 0x22 request: 22 10 00 10 01 (5 bytes) */
    req[0] = SYN_UDS_SID_READ_DATA_BY_IDENTIFIER;
    syn_poke_u16(0x1000, req, 1);
    syn_poke_u16(0x1001, req, 3);

    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 5, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x62, resp[0]);
    TEST_ASSERT_EQUAL(11U, resp_len); /* 1 + 2 + 2 + 2 + 4 */

    /* Verify DID 0x1000 */
    TEST_ASSERT_EQUAL_HEX16(0x1000, syn_peek_u16(resp, 1));
    TEST_ASSERT_EQUAL_HEX8(0xAA, resp[3]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, resp[4]);

    /* Verify DID 0x1001 */
    TEST_ASSERT_EQUAL_HEX16(0x1001, syn_peek_u16(resp, 5));
    TEST_ASSERT_EQUAL_HEX8(0x11, resp[7]);
    TEST_ASSERT_EQUAL_HEX8(0x22, resp[8]);
    TEST_ASSERT_EQUAL_HEX8(0x33, resp[9]);
    TEST_ASSERT_EQUAL_HEX8(0x44, resp[10]);

    /* 2. Malformed request length (4 bytes: 22 10 00 10) -> NRC 0x13 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req, 4, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
}

void test_uds_memory_by_address_integer_overflow(void)
{
    syn_uds_init(&g_uds);
    g_uds.security_state = SYN_UDS_SECURITY_UNLOCKED;

    uint8_t resp[64] = {0};
    uint16_t resp_len = 0;

    /* 1. ReadMemoryByAddress with size = 0xFFFFFFFF (alfid = 0x44: 4B addr, 4B size) */
    uint8_t req_read[10] = {
        SYN_UDS_SID_READ_MEMORY_BY_ADDRESS, 0x44, 0x00, 0x00, 0x10, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_read, sizeof(req_read), resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    /* 2. WriteMemoryByAddress with size = 0xFFFFFFFF (alfid = 0x44: 4B addr, 4B size) */
    uint8_t req_write[12] = {SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS,
                             0x44,
                             0x00,
                             0x00,
                             0x10,
                             0x00,
                             0xFF,
                             0xFF,
                             0xFF,
                             0xFF,
                             0x01,
                             0x02};
    TEST_ASSERT_TRUE(syn_uds_process_request(&g_uds, req_write, sizeof(req_write), resp,
                                             sizeof(resp), &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_RESPONSE_NEGATIVE, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);
}

void test_r5_a05_security_session_dtc_and_transfer_fixes(void)
{
    SYN_UDS_Server srv;
    TEST_ASSERT_TRUE(syn_uds_init(&srv));
    TEST_ASSERT_TRUE(srv.dtc_setting_enabled);

    uint8_t did_buf[4] = {0x10U, 0x20U, 0x30U, 0x40U};
    TEST_ASSERT_TRUE(syn_uds_register_did_ext(
        &srv, 0x0100U, did_buf, 4U, true, SYN_UDS_SESSION_MASK_ALL, SYN_UDS_SECURITY_MASK_LEVEL_1));
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&srv, 0x012345U, 0x00U, 0x80U));
    TEST_ASSERT_TRUE(syn_uds_register_dtc(&srv, 0x412345U, 0x00U, 0x40U));

    uint8_t req[32];
    uint8_t resp[64];
    uint16_t resp_len = 0U;

    /* 1. Brute-force lockout (3 bad keys) persists across 0x10 session changes */
    for (uint8_t attempt = 0U; attempt < 3U; attempt++) {
        req[0] = SYN_UDS_SID_SECURITY_ACCESS;
        req[1] = 0x01U;
        TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x67U, resp[0]);

        req[1] = 0x02U;
        syn_poke_u32(0xDEADBEEFU, req, 2);
        TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 6U, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    }
    TEST_ASSERT_EQUAL_UINT32(SYN_UDS_SECURITY_DELAY_MS, srv.security_delay_timer_ms);

    const uint8_t sessions[3] = {SYN_UDS_SESSION_EXTENDED, SYN_UDS_SESSION_PROGRAMMING,
                                 SYN_UDS_SESSION_DEFAULT};
    for (size_t i = 0U; i < 3U; i++) {
        uint8_t sess_req[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, sessions[i]};
        TEST_ASSERT_TRUE(syn_uds_process_request(&srv, sess_req, 2U, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x50U, resp[0]);
        TEST_ASSERT_EQUAL_UINT32(SYN_UDS_SECURITY_DELAY_MS, srv.security_delay_timer_ms);
        TEST_ASSERT_EQUAL_UINT8(SYN_UDS_SECURITY_MAX_ATTEMPTS, srv.security_error_count);

        req[0] = SYN_UDS_SID_SECURITY_ACCESS;
        req[1] = 0x01U;
        TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                                 SYN_UDS_ADDR_PHYSICAL));
        TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
        TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED, resp[2]);
    }

    /* Expire delay timer */
    syn_uds_tick(&srv, SYN_UDS_SECURITY_DELAY_MS);
    TEST_ASSERT_EQUAL_UINT32(0U, srv.security_delay_timer_ms);

    /* 2. Unlocking Level 1 relocks on same-session re-entry, cross-session, and S3 timeout */
    uint8_t sess_ext[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_EXTENDED};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, sess_ext, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01U;
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    req[1] = 0x02U;
    syn_poke_u32(srv.active_seed ^ 0xA5A5A5A5U, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 6U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, srv.security_state);
    TEST_ASSERT_EQUAL_UINT8(1U, srv.security_level);

    /* Same-session re-entry (EXTENDED -> EXTENDED) relocks per ISO 14229-1 9.2.2 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, sess_ext, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, srv.security_state);
    TEST_ASSERT_EQUAL_UINT8(0U, srv.security_level);

    /* Unlock again, then cross-session EXTENDED -> PROGRAMMING relocks */
    req[1] = 0x01U;
    (void)syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02U;
    syn_poke_u32(srv.active_seed ^ 0xA5A5A5A5U, req, 2);
    (void)syn_uds_process_request(&srv, req, 6U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_UNLOCKED, srv.security_state);

    uint8_t sess_prog[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, SYN_UDS_SESSION_PROGRAMMING};
    (void)syn_uds_process_request(&srv, sess_prog, 2U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, srv.security_state);
    TEST_ASSERT_EQUAL_UINT8(0U, srv.security_level);

    /* 3. High security level (> 15) must not trigger shift UB and denies Level-1 DID */
    req[1] = 0x41U;
    (void)syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x42U;
    syn_poke_u32(srv.active_seed ^ 0xA5A5A5A5U, req, 2);
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req, 6U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_UINT8(33U, srv.security_level);

    uint8_t rdbi[3] = {SYN_UDS_SID_READ_DATA_BY_IDENTIFIER, 0x01U, 0x00U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, rdbi, 3U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp[2]);

    /* 4. 0x85 ControlDTCSetting gates syn_uds_dtc_report_test_result & resets on S3 timeout */
    uint8_t dtc_off[2] = {SYN_UDS_SID_CONTROL_DTC_SETTING, 0x02U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, dtc_off, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0xC5U, resp[0]);
    TEST_ASSERT_FALSE(srv.dtc_setting_enabled);
    TEST_ASSERT_FALSE(syn_uds_dtc_report_test_result(&srv, 0x012345U, true));
    TEST_ASSERT_EQUAL_HEX8(0x00U, srv.dtc_table[0].status);

    uint8_t dtc_on[2] = {SYN_UDS_SID_CONTROL_DTC_SETTING, 0x01U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, dtc_on, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_TRUE(srv.dtc_setting_enabled);
    TEST_ASSERT_TRUE(syn_uds_dtc_report_test_result(&srv, 0x012345U, true));
    TEST_ASSERT_TRUE(syn_uds_dtc_report_test_result(&srv, 0x412345U, true));
    TEST_ASSERT_NOT_EQUAL(0x00U, srv.dtc_table[0].status);

    /* Disable again, then S3 timeout restores DEFAULT session, locks security & re-enables DTCs */
    (void)syn_uds_process_request(&srv, dtc_off, 2U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    TEST_ASSERT_FALSE(srv.dtc_setting_enabled);
    syn_uds_tick(&srv, SYN_UDS_S3_TIMEOUT_MS);
    TEST_ASSERT_EQUAL(SYN_UDS_SESSION_DEFAULT, srv.session);
    TEST_ASSERT_EQUAL(SYN_UDS_SECURITY_LOCKED, srv.security_state);
    TEST_ASSERT_EQUAL_UINT8(0U, srv.security_level);
    TEST_ASSERT_TRUE(srv.dtc_setting_enabled);

    /* 5. 0x14 ClearDiagnosticInformation clears status/fault_cnt without deleting dtc_table */
    uint8_t clr_one[4] = {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 0x01U, 0x23U, 0x45U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, clr_one, 4U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x54U, resp[0]);
    TEST_ASSERT_EQUAL_UINT8(2U, srv.dtc_count);
    TEST_ASSERT_EQUAL_HEX8(0x50U, srv.dtc_table[0].status);
    TEST_ASSERT_EQUAL_INT8(0, srv.dtc_table[0].fault_cnt);
    TEST_ASSERT_NOT_EQUAL(0x50U, srv.dtc_table[1].status);

    /* 0x19 0x0A ReportSupportedDTC still returns both registered DTCs */
    uint8_t rep_sup[2] = {SYN_UDS_SID_READ_DTC_INFORMATION, SYN_UDS_DTC_REPORT_SUPPORTED};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, rep_sup, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x59U, resp[0]);
    TEST_ASSERT_EQUAL_UINT16(11U, resp_len);

    /* 6. 0x2E exact length check (short & overlong both return NRC 0x13) */
    req[0] = SYN_UDS_SID_SECURITY_ACCESS;
    req[1] = 0x01U;
    (void)syn_uds_process_request(&srv, req, 2U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    req[1] = 0x02U;
    syn_poke_u32(srv.active_seed ^ 0xA5A5A5A5U, req, 2);
    (void)syn_uds_process_request(&srv, req, 6U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);

    uint8_t wdbi[10] = {SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER, 0x01U, 0x00U, 1U, 2U, 3U, 4U, 5U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, wdbi, 5U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, wdbi, 8U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, wdbi, 7U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x6EU, resp[0]);

    /* 7. 0x36 block_seq 0x00 rejected when transfer_bytes_processed == 0; retransmit works > 0 */
    uint8_t req_dl[5] = {SYN_UDS_SID_REQUEST_DOWNLOAD, 0x00U, 0x11U, 0x10U, 0x08U};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&srv, req_dl, 5U, resp, 3U, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, req_dl, 5U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x74U, resp[0]);

    uint8_t td_zero[4] = {SYN_UDS_SID_TRANSFER_DATA, 0x00U, 0xAAU, 0xBBU};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, td_zero, 4U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x7FU, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER, resp[2]);

    td_zero[1] = 0x01U;
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, td_zero, 4U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76U, resp[0]);
    /* Retransmit block 0x01 now that transfer_bytes_processed > 0 */
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, td_zero, 4U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(0x76U, resp[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01U, resp[1]);

    /* Response-too-long branches on upload, secured data, ROE, and timing read + session mask upd
     */
    uint8_t req_ul[5] = {SYN_UDS_SID_REQUEST_UPLOAD, 0x00U, 0x11U, 0x10U, 0x08U};
    (void)syn_uds_process_request(&srv, req_ul, 5U, resp, sizeof(resp), &resp_len,
                                  SYN_UDS_ADDR_PHYSICAL);
    uint8_t td_ul[2] = {SYN_UDS_SID_TRANSFER_DATA, 0x01U};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&srv, td_ul, 2U, resp, 4U, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t sec_tx[4] = {SYN_UDS_SID_SECURED_DATA_TRANSMISSION, 1U, 2U, 3U};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&srv, sec_tx, 4U, resp, 3U, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t roe_req[2] = {SYN_UDS_SID_RESPONSE_ON_EVENT, 0x01U};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&srv, roe_req, 2U, resp, 3U, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t tim_req[2] = {SYN_UDS_SID_ACCESS_TIMING_PARAMETER, SYN_UDS_TIMING_READ_ACTIVE};
    TEST_ASSERT_TRUE(
        syn_uds_process_request(&srv, tim_req, 2U, resp, 5U, &resp_len, SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_RESPONSE_TOO_LONG, resp[2]);

    uint8_t tim_bad_given[3] = {SYN_UDS_SID_ACCESS_TIMING_PARAMETER, SYN_UDS_TIMING_SET_TO_GIVEN,
                                0x00U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, tim_bad_given, 3U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_set_service_session_mask(&srv, 0x28U, SYN_UDS_SESSION_MASK_EXTENDED));
    /* Functional request hitting NRC 0x7E (suppressed) & NRC 0x12 (suppressed) + short 0x38 */
    SYN_UDS_Server func_srv;
    TEST_ASSERT_TRUE(syn_uds_init(&func_srv));
    TEST_ASSERT_TRUE(
        syn_uds_set_service_session_mask(&func_srv, 0x28U, SYN_UDS_SESSION_MASK_EXTENDED));
    uint8_t func_cc[3] = {SYN_UDS_SID_COMMUNICATION_CONTROL, 0x00U, 0x01U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&func_srv, func_cc, 3U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);

    uint8_t func_bad_sub[2] = {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, 0x7FU};
    TEST_ASSERT_TRUE(syn_uds_process_request(&func_srv, func_bad_sub, 2U, resp, sizeof(resp),
                                             &resp_len, SYN_UDS_ADDR_FUNCTIONAL));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);

    uint8_t short_ft[2] = {SYN_UDS_SID_REQUEST_FILE_TRANSFER, 0x01U};
    TEST_ASSERT_TRUE(syn_uds_process_request(&srv, short_ft, 2U, resp, sizeof(resp), &resp_len,
                                             SYN_UDS_ADDR_PHYSICAL));
    TEST_ASSERT_EQUAL_HEX8(SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp[2]);

    TEST_ASSERT_TRUE(syn_uds_set_service_session_mask(&srv, 0x28U, SYN_UDS_SESSION_MASK_ALL));
}

void run_uds_tests(void)
{
    RUN_TEST(test_uds_init_and_sessions);
    RUN_TEST(test_uds_s3_timer_tick);
    RUN_TEST(test_uds_security_access);
    RUN_TEST(test_uds_security_access_aes128);
    RUN_TEST(test_uds_security_access_repeated_unlock);
    RUN_TEST(test_uds_communication_control);
    RUN_TEST(test_uds_access_timing_parameter);
    RUN_TEST(test_uds_secured_data_transmission);
    RUN_TEST(test_uds_extended_sids);
    RUN_TEST(test_uds_complete_27_sids);
    RUN_TEST(test_uds_did_read_write);
    RUN_TEST(test_uds_ecu_reset_routine_tester_present);
    RUN_TEST(test_uds_read_dtc_information_subfunctions);
    RUN_TEST(test_uds_stateful_data_transfer_sequence);
    RUN_TEST(test_uds_spec_nrc_and_edge_cases);
    RUN_TEST(test_uds_upload_sequence_and_nrc_handling);
    RUN_TEST(test_uds_negative_response_codes);
    RUN_TEST(test_uds_negative_response_codes_extended);
    RUN_TEST(test_uds_bounds_and_null_checks);
    RUN_TEST(test_uds_dtc_overflow_and_short_msg_nrcs);
    RUN_TEST(test_uds_read_dtc_by_status_mask);
    RUN_TEST(test_uds_clear_dtc_group_filtering);
    RUN_TEST(test_uds_dtc_iso14229_bit_operations);
    RUN_TEST(test_uds_security_and_routine_error_handling);
    RUN_TEST(test_uds_dtc_and_transfer_boundary_conditions);
    RUN_TEST(test_uds_deferred_reset_callback);
    RUN_TEST(test_uds_session_transition_policy);
    RUN_TEST(test_uds_response_length_exceeded);
    RUN_TEST(test_uds_isotp_full_stack_loopback);
    RUN_TEST(test_uds_session_mask_filtering);
    RUN_TEST(test_uds_security_mask_filtering);
    RUN_TEST(test_uds_addressing_modes);
    RUN_TEST(test_uds_custom_service_policies);
    RUN_TEST(test_uds_iso14229_service_callbacks);
    RUN_TEST(test_uds_multi_did_read);
    RUN_TEST(test_uds_memory_by_address_integer_overflow);
    RUN_TEST(test_r5_a05_security_session_dtc_and_transfer_fixes);
}
