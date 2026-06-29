/**
 * @file test_ringbuf.c
 * @brief Unity tests for syn_ringbuf.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/util/syn_ringbuf.h"

static void test_ringbuf_init_empty(void)
{
    uint8_t buf[16];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
    TEST_ASSERT_FALSE(syn_ringbuf_full(&rb));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_count(&rb));
    /* N-1 usable slots in a classic ring buffer */
    TEST_ASSERT_EQUAL_size_t(15, syn_ringbuf_free(&rb));
}

static void test_ringbuf_put_get(void)
{
    uint8_t buf[16];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    TEST_ASSERT_TRUE(syn_ringbuf_put(&rb, 0x42));
    TEST_ASSERT_EQUAL_size_t(1, syn_ringbuf_count(&rb));

    uint8_t val = 0;
    TEST_ASSERT_TRUE(syn_ringbuf_get(&rb, &val));
    TEST_ASSERT_EQUAL_HEX8(0x42, val);
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

static void test_ringbuf_fill_and_overflow(void)
{
    uint8_t buf[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* Fill it (7 usable slots) */
    for (uint8_t i = 0; i < 7; i++) {
        TEST_ASSERT_TRUE(syn_ringbuf_put(&rb, (uint8_t)(0xA0 + i)));
    }
    TEST_ASSERT_TRUE(syn_ringbuf_full(&rb));

    /* Overflow returns false */
    TEST_ASSERT_FALSE(syn_ringbuf_put(&rb, 0xFF));

    /* Drain in order */
    for (uint8_t i = 0; i < 7; i++) {
        uint8_t val;
        TEST_ASSERT_TRUE(syn_ringbuf_get(&rb, &val));
        TEST_ASSERT_EQUAL_HEX8(0xA0 + i, val);
    }
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

static void test_ringbuf_get_empty(void)
{
    uint8_t buf[4];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t val;
    TEST_ASSERT_FALSE(syn_ringbuf_get(&rb, &val));
}

static void test_ringbuf_peek(void)
{
    uint8_t buf[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    syn_ringbuf_put(&rb, 0xAA);
    syn_ringbuf_put(&rb, 0xBB);

    uint8_t val;
    TEST_ASSERT_TRUE(syn_ringbuf_peek(&rb, &val));
    TEST_ASSERT_EQUAL_HEX8(0xAA, val);
    /* Peek doesn't consume */
    TEST_ASSERT_EQUAL_size_t(2, syn_ringbuf_count(&rb));
}

static void test_ringbuf_wraparound(void)
{
    uint8_t buf[4];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* Fill and drain to advance pointers */
    syn_ringbuf_put(&rb, 0xA0);
    syn_ringbuf_put(&rb, 0xA1);
    uint8_t tmp;
    syn_ringbuf_get(&rb, &tmp);
    syn_ringbuf_get(&rb, &tmp);

    /* Now write across the wrap boundary (3 usable slots) */
    syn_ringbuf_put(&rb, 0xB0);
    syn_ringbuf_put(&rb, 0xB1);
    syn_ringbuf_put(&rb, 0xB2);

    TEST_ASSERT_TRUE(syn_ringbuf_full(&rb));

    syn_ringbuf_get(&rb, &tmp);
    TEST_ASSERT_EQUAL_HEX8(0xB0, tmp);
    syn_ringbuf_get(&rb, &tmp);
    TEST_ASSERT_EQUAL_HEX8(0xB1, tmp);
    syn_ringbuf_get(&rb, &tmp);
    TEST_ASSERT_EQUAL_HEX8(0xB2, tmp);
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

static void test_ringbuf_reset(void)
{
    uint8_t buf[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    syn_ringbuf_put(&rb, 1);
    syn_ringbuf_put(&rb, 2);
    syn_ringbuf_reset(&rb);

    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_count(&rb));
}

static void test_ringbuf_write_read_bulk(void)
{
    uint8_t buf[32];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t data[] = {1, 2, 3, 4, 5};
    TEST_ASSERT_EQUAL_size_t(5, syn_ringbuf_write(&rb, data, 5));
    TEST_ASSERT_EQUAL_size_t(5, syn_ringbuf_count(&rb));

    uint8_t out[5] = {0};
    TEST_ASSERT_EQUAL_size_t(5, syn_ringbuf_read(&rb, out, 5));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, 5);
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

static void test_ringbuf_bulk_wraparound(void)
{
    uint8_t buf[8]; /* 7 usable slots */
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* Advance the pointers to near the end */
    uint8_t tmp[5] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4};
    syn_ringbuf_write(&rb, tmp, 5);
    uint8_t drain[5];
    syn_ringbuf_read(&rb, drain, 5);

    /* Now head and tail are at index 5. Write 6 bytes: wraps around. */
    uint8_t data[] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5};
    TEST_ASSERT_EQUAL_size_t(6, syn_ringbuf_write(&rb, data, 6));

    uint8_t out[6] = {0};
    TEST_ASSERT_EQUAL_size_t(6, syn_ringbuf_read(&rb, out, 6));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, 6);
}

static void test_ringbuf_bulk_partial_write(void)
{
    uint8_t buf[4]; /* 3 usable slots */
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t data[] = {1, 2, 3, 4, 5};
    /* Only 3 bytes fit */
    TEST_ASSERT_EQUAL_size_t(3, syn_ringbuf_write(&rb, data, 5));
    TEST_ASSERT_TRUE(syn_ringbuf_full(&rb));

    uint8_t out[3] = {0};
    TEST_ASSERT_EQUAL_size_t(3, syn_ringbuf_read(&rb, out, 3));
    TEST_ASSERT_EQUAL_HEX8(1, out[0]);
    TEST_ASSERT_EQUAL_HEX8(2, out[1]);
    TEST_ASSERT_EQUAL_HEX8(3, out[2]);
}

static void test_ringbuf_bulk_partial_read(void)
{
    uint8_t buf[16];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t data[] = {10, 20, 30};
    syn_ringbuf_write(&rb, data, 3);

    /* Ask for more than available */
    uint8_t out[8] = {0};
    TEST_ASSERT_EQUAL_size_t(3, syn_ringbuf_read(&rb, out, 8));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, 3);
}

void run_ringbuf_tests(void)
{
    RUN_TEST(test_ringbuf_init_empty);
    RUN_TEST(test_ringbuf_put_get);
    RUN_TEST(test_ringbuf_fill_and_overflow);
    RUN_TEST(test_ringbuf_get_empty);
    RUN_TEST(test_ringbuf_peek);
    RUN_TEST(test_ringbuf_wraparound);
    RUN_TEST(test_ringbuf_reset);
    RUN_TEST(test_ringbuf_write_read_bulk);
    RUN_TEST(test_ringbuf_bulk_wraparound);
    RUN_TEST(test_ringbuf_bulk_partial_write);
    RUN_TEST(test_ringbuf_bulk_partial_read);
}

