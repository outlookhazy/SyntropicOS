/**
 * @file test_rc_failsafe.c
 * @brief Unit tests for Zero-Heap RC Safety Failsafe Manager.
 */

#include "syntropic/control/syn_rc_failsafe.h"
#include "unity/unity.h"

void test_rc_failsafe_init(void)
{
    SYN_Failsafe_Config cfg = {.timeout_ms = 500,
                               .channel_modes = {SYN_FAILSAFE_DISARM, SYN_FAILSAFE_HOLD},
                               .fallback_us = {1000, 1500}};

    SYN_Failsafe_Manager mgr;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_failsafe_init(&mgr, &cfg));
    TEST_ASSERT_TRUE(mgr.in_failsafe);
    TEST_ASSERT_FALSE(mgr.ever_received);
    TEST_ASSERT_EQUAL_UINT16(1000U, mgr.channels[0]);
    TEST_ASSERT_EQUAL_UINT16(1500U, mgr.channels[1]);
    TEST_ASSERT_EQUAL_UINT16(1000U, mgr.channels[2]); /* Throttle default safe idle */
}

void test_rc_failsafe_timeout_trigger(void)
{
    SYN_Failsafe_Config cfg = {
        .timeout_ms = 500,
        .channel_modes = {SYN_FAILSAFE_DISARM, SYN_FAILSAFE_FALLBACK, SYN_FAILSAFE_HOLD},
        .fallback_us = {1000, 1200, 1500}};

    SYN_Failsafe_Manager mgr;
    syn_failsafe_init(&mgr, &cfg);

    uint16_t in_ch[3] = {1800, 1600, 1400};
    syn_failsafe_feed_frame(&mgr, in_ch, 3, 1000);

    uint16_t out_ch[16];
    bool is_failsafe = syn_failsafe_step(&mgr, 1200, out_ch);
    TEST_ASSERT_FALSE(is_failsafe);
    TEST_ASSERT_EQUAL_UINT16(1800, out_ch[0]);

    /* Advance 600 ms -> Timeout! */
    is_failsafe = syn_failsafe_step(&mgr, 1800, out_ch);
    TEST_ASSERT_TRUE(is_failsafe);
    TEST_ASSERT_EQUAL_UINT16(1000, out_ch[0]); /* Disarmed */
    TEST_ASSERT_EQUAL_UINT16(1200, out_ch[1]); /* Fallback */
    TEST_ASSERT_EQUAL_UINT16(1400, out_ch[2]); /* Hold */
}

void test_rc_failsafe_null_and_error(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_failsafe_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_failsafe_feed_frame(NULL, NULL, 0, 0));
    TEST_ASSERT_TRUE(syn_failsafe_step(NULL, 1000, NULL));

    /* Channel count overflow clamping test (> 16 channels) */
    SYN_Failsafe_Config cfg = {.timeout_ms = 500};
    SYN_Failsafe_Manager mgr;
    syn_failsafe_init(&mgr, &cfg);
    uint16_t oversized_ch[32];
    for (int i = 0; i < 32; i++) {
        oversized_ch[i] = 1600;
    }
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_failsafe_feed_frame(&mgr, oversized_ch, 0, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_failsafe_feed_frame(&mgr, oversized_ch, 32, 1000));
}

void test_r5_rc_failsafe_regressions(void)
{
    /* Cold-boot without TX must remain disarmed in failsafe even before timeout_ms elapses */
    SYN_Failsafe_Config cfg = {.timeout_ms = 500};
    cfg.channel_modes[3] = SYN_FAILSAFE_FALLBACK;
    SYN_Failsafe_Manager mgr;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_failsafe_init(&mgr, &cfg));

    uint16_t out_ch[SYN_RC_FAILSAFE_MAX_CHANNELS];
    TEST_ASSERT_TRUE(syn_failsafe_step(&mgr, 10U, out_ch));
    TEST_ASSERT_EQUAL_UINT16(1500U, out_ch[0]);
    TEST_ASSERT_EQUAL_UINT16(1500U, out_ch[1]);
    TEST_ASSERT_EQUAL_UINT16(1000U, out_ch[2]); /* Throttle stays at 1000 us on cold boot */
    TEST_ASSERT_EQUAL_UINT16(1500U, out_ch[3]);
    TEST_ASSERT_EQUAL_UINT32(0U, mgr.failsafe_events);

    /* Step with NULL out_channels while in failsafe */
    TEST_ASSERT_TRUE(syn_failsafe_step(&mgr, 20U, NULL));

    /* First valid frame arms RC output; subsequent timeout triggers failsafe_events == 1 */
    uint16_t rc_frame[4] = {1510U, 1490U, 1350U, 1505U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_failsafe_feed_frame(&mgr, rc_frame, 4U, 100U));
    TEST_ASSERT_FALSE(syn_failsafe_step(&mgr, 200U, out_ch));
    TEST_ASSERT_EQUAL_UINT16(1350U, out_ch[2]);

    TEST_ASSERT_TRUE(syn_failsafe_step(&mgr, 700U, out_ch));
    TEST_ASSERT_EQUAL_UINT32(1U, mgr.failsafe_events);
    /* Second step in failsafe does not double-increment failsafe_events */
    TEST_ASSERT_TRUE(syn_failsafe_step(&mgr, 800U, out_ch));
    TEST_ASSERT_EQUAL_UINT32(1U, mgr.failsafe_events);
}
