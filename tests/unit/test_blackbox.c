/**
 * @file test_blackbox.c
 * @brief Unit tests for Zero-Heap Flight Telemetry Blackbox Binary Recorder.
 */

#include "syntropic/log/syn_blackbox.h"
#include "unity/unity.h"

#include <limits.h>
#include <string.h>

static uint64_t decode_uvarint(const uint8_t *buf, size_t *pos)
{
    uint64_t val = 0ULL;
    uint32_t shift = 0U;
    while (true) {
        uint8_t b = buf[(*pos)++];
        val |= ((uint64_t)(b & 0x7FU)) << shift;
        if ((b & 0x80U) == 0U) {
            break;
        }
        shift += 7U;
    }
    return val;
}

static int64_t decode_zigzag64(const uint8_t *buf, size_t *pos)
{
    uint64_t zz = decode_uvarint(buf, pos);
    return ((zz & 1ULL) != 0ULL) ? (-(int64_t)(zz >> 1U) - 1LL) : (int64_t)(zz >> 1U);
}

void test_blackbox_init(void)
{
    SYN_Blackbox bb;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_blackbox_init(&bb));
    TEST_ASSERT_EQUAL_UINT32(0, bb.frame_count);
}

void test_blackbox_varint(void)
{
    uint8_t buf[8];
    size_t len = syn_blackbox_encode_varint(0, buf);
    TEST_ASSERT_EQUAL_INT(1, len);
    TEST_ASSERT_EQUAL_UINT8(0, buf[0]);

    len = syn_blackbox_encode_varint(-1, buf);
    TEST_ASSERT_EQUAL_INT(1, len);
    TEST_ASSERT_EQUAL_UINT8(1, buf[0]); /* ZigZag(-1) = 1 */
}

void test_blackbox_encode_intra_and_delta(void)
{
    SYN_Blackbox bb;
    syn_blackbox_init(&bb);

    SYN_Blackbox_Record rec1 = {.iteration = 100,
                                .time_us = 10000,
                                .gyro = {10, -5, 2},
                                .accel = {0, 0, 1000},
                                .setpoint = {0, 0, 0, 1500},
                                .motor = {1500, 1500, 1500, 1500}};

    uint8_t buf[64];
    size_t out_len = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_blackbox_encode_intra(&bb, &rec1, buf, &out_len));
    TEST_ASSERT_EQUAL_UINT8('I', buf[0]);
    TEST_ASSERT_TRUE(out_len > 15);
    TEST_ASSERT_EQUAL_UINT32(1, bb.frame_count);

    SYN_Blackbox_Record rec2 = rec1;
    rec2.iteration++;
    rec2.time_us += 1000;
    rec2.gyro[0] += 2;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_blackbox_encode_delta(&bb, &rec2, buf, &out_len));
    TEST_ASSERT_EQUAL_UINT8('P', buf[0]);
    TEST_ASSERT_TRUE(out_len > 10);
    TEST_ASSERT_EQUAL_UINT32(2, bb.frame_count);
}

void test_blackbox_null_checks(void)
{
    SYN_Blackbox bb;
    SYN_Blackbox_Record rec = {0};
    uint8_t buf[SYN_BLACKBOX_MAX_FRAME_SIZE];
    size_t out_len = 0U;

    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_blackbox_encode_varint(0, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_intra(NULL, &rec, buf, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_intra(&bb, NULL, buf, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_intra(&bb, &rec, NULL, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_intra(&bb, &rec, buf, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_delta(NULL, &rec, buf, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_delta(&bb, NULL, buf, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_delta(&bb, &rec, NULL, &out_len));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_blackbox_encode_delta(&bb, &rec, buf, NULL));
}

void test_r5_blackbox_regressions(void)
{
    /* 1. INT32_MIN and INT32_MAX varint round-trip within <= 5 bytes */
    uint8_t vbuf[8];
    size_t pos = 0U;
    size_t n = syn_blackbox_encode_varint(INT32_MIN, vbuf);
    TEST_ASSERT_EQUAL_UINT32(5U, (uint32_t)n);
    TEST_ASSERT_EQUAL_INT64((int64_t)INT32_MIN, decode_zigzag64(vbuf, &pos));

    pos = 0U;
    n = syn_blackbox_encode_varint(INT32_MAX, vbuf);
    TEST_ASSERT_EQUAL_UINT32(5U, (uint32_t)n);
    TEST_ASSERT_EQUAL_INT64((int64_t)INT32_MAX, decode_zigzag64(vbuf, &pos));

    /* 2. R5-S05: time_us = 3000000000U (> 35.8 min) and worst-case 16-field extrema fit <= 64B */
    SYN_Blackbox bb;
    TEST_ASSERT_EQUAL(SYN_OK, syn_blackbox_init(&bb));

    SYN_Blackbox_Record rec_hi = {.iteration = 0xFFFFFFFFU,
                                  .time_us = 3000000000U,
                                  .gyro = {-32768, 32767, -32768},
                                  .accel = {32767, -32768, 32767},
                                  .setpoint = {-32768, 32767, -32768, 32767},
                                  .motor = {65535U, 65535U, 65535U, 65535U}};

    uint8_t frame[SYN_BLACKBOX_MAX_FRAME_SIZE];
    size_t out_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_blackbox_encode_intra(&bb, &rec_hi, frame, &out_len));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(SYN_BLACKBOX_MAX_FRAME_SIZE, (uint32_t)out_len);
    TEST_ASSERT_EQUAL_UINT8('I', frame[0]);

    pos = 1U;
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFULL, decode_uvarint(frame, &pos));
    TEST_ASSERT_EQUAL_UINT64(3000000000ULL, decode_uvarint(frame, &pos));

    /* Worst-case opposite-sign P-frame delta + large >2^31 timestamp jump + negative delta */
    SYN_Blackbox_Record rec_lo = {.iteration = 0U,
                                  .time_us = 1000U,
                                  .gyro = {32767, -32768, 32767},
                                  .accel = {-32768, 32767, -32768},
                                  .setpoint = {32767, -32768, 32767, -32768},
                                  .motor = {0U, 0U, 0U, 0U}};

    TEST_ASSERT_EQUAL(SYN_OK, syn_blackbox_encode_delta(&bb, &rec_lo, frame, &out_len));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(SYN_BLACKBOX_MAX_FRAME_SIZE, (uint32_t)out_len);
    TEST_ASSERT_EQUAL_UINT8('P', frame[0]);

    pos = 1U;
    TEST_ASSERT_EQUAL_INT64(-4294967295LL, decode_zigzag64(frame, &pos));
    TEST_ASSERT_EQUAL_INT64(1000LL - 3000000000LL, decode_zigzag64(frame, &pos));

    /* Positive >2^31 time_us jump from 1000U -> 3000000000U never wraps negative */
    TEST_ASSERT_EQUAL(SYN_OK, syn_blackbox_encode_delta(&bb, &rec_hi, frame, &out_len));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(SYN_BLACKBOX_MAX_FRAME_SIZE, (uint32_t)out_len);
    pos = 1U;
    TEST_ASSERT_EQUAL_INT64(4294967295LL, decode_zigzag64(frame, &pos));
    TEST_ASSERT_EQUAL_INT64(3000000000LL - 1000LL, decode_zigzag64(frame, &pos));

    /* Bounded _ex API rejects undersized buffers without mutating state or overflowing canary */
    uint8_t guarded[36];
    memset(guarded, 0xA5, sizeof(guarded));
    uint32_t prev_frames = bb.frame_count;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_blackbox_encode_intra_ex(&bb, &rec_hi, guarded, 32U, &out_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_blackbox_encode_delta_ex(&bb, &rec_lo, guarded, 32U, &out_len));
    TEST_ASSERT_EQUAL_UINT32(prev_frames, bb.frame_count);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, guarded[32]);
}
