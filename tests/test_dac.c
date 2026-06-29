/**
 * @file test_dac.c
 * @brief Unity tests for syn_dac (DAC driver).
 *
 * Mock DAC: 12-bit resolution, 3300 mV reference.
 * max_raw = 4095.  1 LSB = 3300/4095 ~= 0.806 mV.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_dac.h"

#define DAC_CH 0u

static SYN_DAC dac;

/* ── test 1: init ───────────────────────────────────────────────────────── */

static void test_dac_init_ok(void)
{
    mock_dac_init_ok = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_init(&dac, DAC_CH));
    TEST_ASSERT_EQUAL_UINT8(DAC_CH, dac.channel);
}

static void test_dac_init_fail(void)
{
    mock_dac_init_ok = false;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_dac_init(&dac, DAC_CH));
}

/* ── test 2: max_raw = 2^12 - 1 = 4095 ─────────────────────────────────── */

static void test_dac_max_raw(void)
{
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL_UINT16(4095u, syn_dac_max_raw(&dac));
}

/* ── test 3: write_raw stores value ─────────────────────────────────────── */

static void test_dac_write_raw(void)
{
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_raw(&dac, 2048u));
    TEST_ASSERT_EQUAL_UINT16(2048u, mock_dac_values[DAC_CH]);
}

/* ── test 4: write_mv scales correctly ──────────────────────────────────── */

static void test_dac_write_mv_half(void)
{
    /* 1650 mV out of 3300 mV ref -> raw = 1650*4095/3300 = 2047 */
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_mv(&dac, 1650u));
    TEST_ASSERT_EQUAL_UINT16(2047u, mock_dac_values[DAC_CH]);
}

static void test_dac_write_mv_full(void)
{
    /* 3300 mV -> raw = 4095 */
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_mv(&dac, 3300u));
    TEST_ASSERT_EQUAL_UINT16(4095u, mock_dac_values[DAC_CH]);
}

static void test_dac_write_mv_clamp(void)
{
    /* mv > ref_mv -> clamped to 3300 -> raw = 4095 */
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_mv(&dac, 5000u));
    TEST_ASSERT_EQUAL_UINT16(4095u, mock_dac_values[DAC_CH]);
}

/* ── test 5: write_percent ──────────────────────────────────────────────── */

static void test_dac_write_percent_zero(void)
{
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_percent(&dac, 0u));
    TEST_ASSERT_EQUAL_UINT16(0u, mock_dac_values[DAC_CH]);
}

static void test_dac_write_percent_full(void)
{
    /* 100% -> raw = 4095 */
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_percent(&dac, 100u));
    TEST_ASSERT_EQUAL_UINT16(4095u, mock_dac_values[DAC_CH]);
}

static void test_dac_write_percent_clamp(void)
{
    /* >100 clamped to 100 */
    mock_dac_init_ok = true;
    syn_dac_init(&dac, DAC_CH);
    TEST_ASSERT_EQUAL(SYN_OK, syn_dac_write_percent(&dac, 150u));
    TEST_ASSERT_EQUAL_UINT16(4095u, mock_dac_values[DAC_CH]);
}

/* ── Registration ────────────────────────────────────────────────────────── */

void run_dac_tests(void)
{
    RUN_TEST(test_dac_init_ok);
    RUN_TEST(test_dac_init_fail);
    RUN_TEST(test_dac_max_raw);
    RUN_TEST(test_dac_write_raw);
    RUN_TEST(test_dac_write_mv_half);
    RUN_TEST(test_dac_write_mv_full);
    RUN_TEST(test_dac_write_mv_clamp);
    RUN_TEST(test_dac_write_percent_zero);
    RUN_TEST(test_dac_write_percent_full);
    RUN_TEST(test_dac_write_percent_clamp);
}
