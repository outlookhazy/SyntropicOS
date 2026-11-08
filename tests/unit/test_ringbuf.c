#if (defined(__linux__) || defined(__unix__) || defined(__APPLE__)) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * @file test_ringbuf.c
 * @brief Unity tests for syn_ringbuf — full coverage.
 */

#include "mocks/mock_port.h"
#include "syntropic/util/syn_ringbuf.h"
#include "unity/unity.h"

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
#include <sched.h>
#endif

#include <string.h>

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

/** peek on empty buffer — exercises line 79: return false */
static void test_ringbuf_peek_empty(void)
{
    uint8_t buf[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t val;
    TEST_ASSERT_FALSE(syn_ringbuf_peek(&rb, &val));
}

/** write on FULL buffer (len clamped to 0) — exercises line 128 */
static void test_ringbuf_write_full(void)
{
    uint8_t buf[4]; /* 3 usable slots */
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t data[] = {1, 2, 3};
    syn_ringbuf_write(&rb, data, 3); /* fills it */
    TEST_ASSERT_TRUE(syn_ringbuf_full(&rb));

    /* Writing to full buffer — avail=0, len clamped to 0 → return 0 */
    uint8_t extra[] = {9};
    size_t written = syn_ringbuf_write(&rb, extra, 1);
    TEST_ASSERT_EQUAL_size_t(0, written);
}

/** read from empty buffer (len clamped to 0) — exercises line 160 */
static void test_ringbuf_read_empty(void)
{
    uint8_t buf[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    uint8_t out[4];
    size_t n = syn_ringbuf_read(&rb, out, 4);
    TEST_ASSERT_EQUAL_size_t(0, n);
}

/** peek_n with len > avail — clamps len; and peek_n on empty (lines 189-192) */
static void test_ringbuf_peek_n(void)
{
    uint8_t buf[16];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* peek_n on empty — returns 0 */
    uint8_t out[8] = {0};
    size_t n = syn_ringbuf_peek_n(&rb, out, 4);
    TEST_ASSERT_EQUAL_size_t(0, n);

    /* Write 3 bytes, peek_n for more — clamps to 3 */
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    syn_ringbuf_write(&rb, data, 3);
    n = syn_ringbuf_peek_n(&rb, out, 8); /* ask for 8, only 3 available */
    TEST_ASSERT_EQUAL_size_t(3, n);
    TEST_ASSERT_EQUAL_HEX8(0xAA, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, out[2]);
    /* Buffer not consumed */
    TEST_ASSERT_EQUAL_size_t(3, syn_ringbuf_count(&rb));
}

/** peek_n across wraparound — exercises lines 205-206 (two-part copy) */
static void test_ringbuf_peek_n_wraparound(void)
{
    uint8_t buf[8]; /* 7 usable slots */
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* Advance pointers to near end */
    uint8_t pad[5] = {1, 2, 3, 4, 5};
    syn_ringbuf_write(&rb, pad, 5);
    uint8_t drain[5];
    syn_ringbuf_read(&rb, drain, 5);
    /* head and tail at index 5 */

    /* Write 6 bytes that will wrap: [5..7] + [0..2] */
    uint8_t data[] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5};
    syn_ringbuf_write(&rb, data, 6);

    /* peek_n should read across the wrap boundary */
    uint8_t out[6] = {0};
    size_t n = syn_ringbuf_peek_n(&rb, out, 6);
    TEST_ASSERT_EQUAL_size_t(6, n);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, 6);
    /* Data still in buffer */
    TEST_ASSERT_EQUAL_size_t(6, syn_ringbuf_count(&rb));
}

/**
 * Validates that acquire/release barriers are actually invoked
 * during ring buffer operations (compiled with SYN_USE_MULTICORE=1).
 *
 * This ensures the barrier instrumentation is wired up correctly —
 * not just that the functional behavior is unchanged.
 */
static void test_ringbuf_barriers_invoked(void)
{
#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    extern uint32_t mock_barrier_count;

    uint8_t buf[8];
    SYN_RingBuf rb;

    /* init should use STORE_RELEASE for head and tail */
    mock_barrier_count = 0;
    syn_ringbuf_init(&rb, buf, sizeof(buf));
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    /* put should invoke barriers for head/tail access */
    mock_barrier_count = 0;
    syn_ringbuf_put(&rb, 0xAA);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    /* get should invoke barriers for head/tail access */
    mock_barrier_count = 0;
    uint8_t val;
    syn_ringbuf_get(&rb, &val);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);
    TEST_ASSERT_EQUAL_HEX8(0xAA, val);

    /* empty/full/count should invoke barriers */
    mock_barrier_count = 0;
    (void)syn_ringbuf_empty(&rb);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    mock_barrier_count = 0;
    (void)syn_ringbuf_full(&rb);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    mock_barrier_count = 0;
    (void)syn_ringbuf_count(&rb);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    /* peek should invoke barriers */
    syn_ringbuf_put(&rb, 0xBB);
    mock_barrier_count = 0;
    (void)syn_ringbuf_peek(&rb, &val);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    /* peek_n should invoke barriers (this was the missed function) */
    uint8_t out[4];
    mock_barrier_count = 0;
    (void)syn_ringbuf_peek_n(&rb, out, sizeof(out));
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    /* bulk write/read should invoke barriers */
    syn_ringbuf_reset(&rb);
    uint8_t data[] = {1, 2, 3};
    mock_barrier_count = 0;
    syn_ringbuf_write(&rb, data, 3);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);

    mock_barrier_count = 0;
    syn_ringbuf_read(&rb, out, 3);
    TEST_ASSERT_TRUE(mock_barrier_count > 0);
#endif
}

static void test_ringbuf_uninitialized_and_zero_len(void)
{
    SYN_RingBuf rb;
    memset(&rb, 0, sizeof(rb));

    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_count(&rb));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_free(&rb));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_write(&rb, (const uint8_t *)"a", 1));

    uint8_t out[4];
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_read(&rb, out, 1));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_peek_n(&rb, out, 1));

    /* Zero len operations */
    uint8_t b[16];
    syn_ringbuf_init(&rb, b, sizeof(b));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_write(&rb, (const uint8_t *)"a", 0));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_read(&rb, out, 0));
    TEST_ASSERT_EQUAL_size_t(0, syn_ringbuf_peek_n(&rb, out, 0));

    /* Test exact array end wrap (new_head == size & new_tail == size) */
    uint8_t b8[8];
    syn_ringbuf_init(&rb, b8, sizeof(b8));
    /* Write 4 bytes: head moves to 4 */
    syn_ringbuf_write(&rb, (const uint8_t *)"1234", 4);
    /* Read 4 bytes: tail moves to 4 */
    syn_ringbuf_read(&rb, out, 4);
    /* Write 4 bytes: head moves 4 + 4 = 8 == rb->size => wraps to 0 */
    syn_ringbuf_write(&rb, (const uint8_t *)"5678", 4);
    TEST_ASSERT_EQUAL_size_t(0, rb.head);
    /* Read 4 bytes: tail moves 4 + 4 = 8 == rb->size => wraps to 0 */
    syn_ringbuf_read(&rb, out, 4);
    TEST_ASSERT_EQUAL_size_t(0, rb.tail);

    /* Test h < t count state explicitly (tail wrapped past head) */
    rb.head = 2;
    rb.tail = 5;
    TEST_ASSERT_EQUAL_size_t(8 - 5 + 2, syn_ringbuf_count(&rb));
}

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define R5_RINGBUF_STRESS_BYTES 50000U

typedef struct {
    SYN_RingBuf *rb;
    size_t written;
    size_t read_total;
    bool seq_ok;
} RingbufStressCtx;

static uint8_t ringbuf_seq_byte(size_t idx)
{
    return (uint8_t)(((idx * 131U) + 17U) & 0xFFU);
}

static void *ringbuf_producer_thread(void *arg)
{
    RingbufStressCtx *ctx = (RingbufStressCtx *)arg;
    uint8_t chunk[13];
    size_t pos = 0U;

    while (pos < R5_RINGBUF_STRESS_BYTES) {
        size_t want = (pos % 13U) + 1U;
        if (pos + want > R5_RINGBUF_STRESS_BYTES) {
            want = R5_RINGBUF_STRESS_BYTES - pos;
        }
        if (want == 1U) {
            if (syn_ringbuf_put(ctx->rb, ringbuf_seq_byte(pos))) {
                pos++;
            } else {
                (void)sched_yield();
            }
        } else {
            for (size_t k = 0U; k < want; k++) {
                chunk[k] = ringbuf_seq_byte(pos + k);
            }
            size_t n = syn_ringbuf_write(ctx->rb, chunk, want);
            if (n == 0U) {
                (void)sched_yield();
            } else {
                pos += n;
            }
        }
    }
    ctx->written = pos;
    return NULL;
}

static void *ringbuf_consumer_thread(void *arg)
{
    RingbufStressCtx *ctx = (RingbufStressCtx *)arg;
    uint8_t chunk[11];
    size_t pos = 0U;

    while (pos < R5_RINGBUF_STRESS_BYTES) {
        size_t want = (pos % 11U) + 1U;
        if (pos + want > R5_RINGBUF_STRESS_BYTES) {
            want = R5_RINGBUF_STRESS_BYTES - pos;
        }
        if (want == 1U) {
            uint8_t b = 0U;
            if (syn_ringbuf_get(ctx->rb, &b)) {
                if (b != ringbuf_seq_byte(pos)) {
                    ctx->seq_ok = false;
                    return NULL;
                }
                pos++;
            } else {
                (void)sched_yield();
            }
        } else {
            size_t n = syn_ringbuf_read(ctx->rb, chunk, want);
            if (n == 0U) {
                (void)sched_yield();
            } else {
                for (size_t k = 0U; k < n; k++) {
                    if (chunk[k] != ringbuf_seq_byte(pos + k)) {
                        ctx->seq_ok = false;
                        return NULL;
                    }
                }
                pos += n;
            }
        }
    }
    ctx->read_total = pos;
    return NULL;
}
#endif

static void test_r5_ringbuf_pthread_stress(void)
{
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    uint8_t backing[64];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, backing, sizeof(backing));

    RingbufStressCtx ctx = {.rb = &rb, .written = 0U, .read_total = 0U, .seq_ok = true};
    pthread_t prod;
    pthread_t cons;

    TEST_ASSERT_EQUAL_INT(0, pthread_create(&prod, NULL, ringbuf_producer_thread, &ctx));
    TEST_ASSERT_EQUAL_INT(0, pthread_create(&cons, NULL, ringbuf_consumer_thread, &ctx));
    TEST_ASSERT_EQUAL_INT(0, pthread_join(prod, NULL));
    TEST_ASSERT_EQUAL_INT(0, pthread_join(cons, NULL));

    TEST_ASSERT_TRUE(ctx.seq_ok);
    TEST_ASSERT_EQUAL_size_t(R5_RINGBUF_STRESS_BYTES, ctx.written);
    TEST_ASSERT_EQUAL_size_t(R5_RINGBUF_STRESS_BYTES, ctx.read_total);
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
#endif
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
    /* New coverage tests */
    RUN_TEST(test_ringbuf_peek_empty);
    RUN_TEST(test_ringbuf_write_full);
    RUN_TEST(test_ringbuf_read_empty);
    RUN_TEST(test_ringbuf_peek_n);
    RUN_TEST(test_ringbuf_peek_n_wraparound);
    /* Barrier validation */
    RUN_TEST(test_ringbuf_barriers_invoked);
    RUN_TEST(test_ringbuf_uninitialized_and_zero_len);
    RUN_TEST(test_r5_ringbuf_pthread_stress);
}
