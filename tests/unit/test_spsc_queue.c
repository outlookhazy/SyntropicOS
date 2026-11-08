#if (defined(__linux__) || defined(__unix__) || defined(__APPLE__)) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * @file test_spsc_queue.c
 * @brief Unit test suite for Lock-Free SPSC Queue (syn_spsc_queue).
 */

#include "syntropic/util/syn_spsc_queue.h"
#include "unity/unity.h"

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
#include <sched.h>
#endif

typedef struct {
    uint32_t id;
    uint16_t val;
} TestMsg;

void test_spsc_queue_push_pop_count(void)
{
    TestMsg buffer[8];
    SYN_SPSC_Queue q;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 8));
    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_full(&q));
    TEST_ASSERT_EQUAL_UINT32(0, syn_spsc_queue_count(&q));

    TestMsg msg1 = {.id = 100, .val = 0xAA};
    TestMsg msg2 = {.id = 101, .val = 0xBB};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &msg1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &msg2));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_EQUAL_UINT32(2, syn_spsc_queue_count(&q));

    TestMsg out;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(100, out.id);
    TEST_ASSERT_EQUAL_UINT16(0xAA, out.val);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(101, out.id);
    TEST_ASSERT_EQUAL_UINT16(0xBB, out.val);

    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(NULL, buffer, sizeof(TestMsg), 8));
}

void test_spsc_queue_null_params_and_capacity(void)
{
    TestMsg buffer[4];
    SYN_SPSC_Queue q;

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(&q, NULL, sizeof(TestMsg), 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(&q, buffer, 0, 4));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 0));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 1));

    /* Minimum valid capacity == 2 (1 usable element + 1 sentinel) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 2));
    TestMsg m_min = {.id = 77U, .val = 88U};
    TestMsg m_out = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &m_min));
    TEST_ASSERT_TRUE(syn_spsc_queue_is_full(&q));
    TEST_ASSERT_EQUAL_UINT32(1U, syn_spsc_queue_count(&q));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_spsc_queue_push(&q, &m_min));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &m_out));
    TEST_ASSERT_EQUAL_UINT32(77U, m_out.id);
    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));

    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(NULL));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_full(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, syn_spsc_queue_count(NULL));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 4));

    TestMsg msg = {.id = 1, .val = 10};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_push(NULL, &msg));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_push(&q, NULL));

    TestMsg out;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_pop(NULL, &out));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spsc_queue_pop(&q, NULL));
}

void test_spsc_queue_full_overflow_and_wraparound(void)
{
    TestMsg buffer[4]; /* Capacity = 4 (3 usable elements) */
    SYN_SPSC_Queue q;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buffer, sizeof(TestMsg), 4));

    TestMsg m1 = {.id = 1, .val = 10};
    TestMsg m2 = {.id = 2, .val = 20};
    TestMsg m3 = {.id = 3, .val = 30};
    TestMsg m4 = {.id = 4, .val = 40};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &m1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &m2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &m3));

    /* Queue is now full (3 items) */
    TEST_ASSERT_TRUE(syn_spsc_queue_is_full(&q));
    TEST_ASSERT_EQUAL_UINT32(3, syn_spsc_queue_count(&q));

    /* Pushing 4th item returns SYN_BUSY */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_spsc_queue_push(&q, &m4));

    /* Pop one item to create space, testing wraparound */
    TestMsg out;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(1, out.id);
    TEST_ASSERT_FALSE(syn_spsc_queue_is_full(&q));

    /* Push m4 successfully now */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_push(&q, &m4));
    TEST_ASSERT_EQUAL_UINT32(3, syn_spsc_queue_count(&q));

    /* Drain remainder */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(2, out.id);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(3, out.id);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_pop(&q, &out));
    TEST_ASSERT_EQUAL_UINT32(4, out.id);

    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
}

static void test_spsc_queue_peek_and_clear(void)
{
    SYN_SPSC_Queue q;
    TestMsg buf[4];
    TEST_ASSERT_EQUAL(SYN_OK, syn_spsc_queue_init(&q, buf, sizeof(TestMsg), 4));
    TEST_ASSERT_FALSE(syn_spsc_queue_is_full(&q));
    TestMsg m1 = {10, 100};
    syn_spsc_queue_push(&q, &m1);
    TEST_ASSERT_EQUAL_size_t(1, syn_spsc_queue_count(&q));
}

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define R5_SPSC_STRESS_ITEMS 50000U

typedef struct {
    SYN_SPSC_Queue *q;
    uint32_t pushed;
    uint32_t popped;
    bool seq_ok;
} SpscStressCtx;

static void *spsc_producer_thread(void *arg)
{
    SpscStressCtx *ctx = (SpscStressCtx *)arg;
    for (uint32_t i = 0U; i < R5_SPSC_STRESS_ITEMS; i++) {
        TestMsg msg;
        msg.id = i;
        msg.val = (uint16_t)(((i * 37U) ^ 0x5A5AU) & 0xFFFFU);
        while (syn_spsc_queue_push(ctx->q, &msg) != SYN_OK) {
            (void)sched_yield();
        }
        ctx->pushed++;
    }
    return NULL;
}

static void *spsc_consumer_thread(void *arg)
{
    SpscStressCtx *ctx = (SpscStressCtx *)arg;
    for (uint32_t i = 0U; i < R5_SPSC_STRESS_ITEMS; i++) {
        TestMsg out = {0U, 0U};
        while (syn_spsc_queue_pop(ctx->q, &out) != SYN_OK) {
            (void)sched_yield();
        }
        uint16_t expected_val = (uint16_t)(((i * 37U) ^ 0x5A5AU) & 0xFFFFU);
        if (out.id != i || out.val != expected_val) {
            ctx->seq_ok = false;
            return NULL;
        }
        ctx->popped++;
    }
    return NULL;
}
#endif

static void test_r5_spsc_queue_pthread_stress(void)
{
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    TestMsg buf[64];
    SYN_SPSC_Queue q;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spsc_queue_init(&q, buf, sizeof(TestMsg), 64U));

    SpscStressCtx ctx = {.q = &q, .pushed = 0U, .popped = 0U, .seq_ok = true};
    pthread_t prod;
    pthread_t cons;

    TEST_ASSERT_EQUAL_INT(0, pthread_create(&prod, NULL, spsc_producer_thread, &ctx));
    TEST_ASSERT_EQUAL_INT(0, pthread_create(&cons, NULL, spsc_consumer_thread, &ctx));
    TEST_ASSERT_EQUAL_INT(0, pthread_join(prod, NULL));
    TEST_ASSERT_EQUAL_INT(0, pthread_join(cons, NULL));

    TEST_ASSERT_TRUE(ctx.seq_ok);
    TEST_ASSERT_EQUAL_UINT32(R5_SPSC_STRESS_ITEMS, ctx.pushed);
    TEST_ASSERT_EQUAL_UINT32(R5_SPSC_STRESS_ITEMS, ctx.popped);
    TEST_ASSERT_TRUE(syn_spsc_queue_is_empty(&q));
#endif
}

void run_spsc_queue_tests(void)
{
    RUN_TEST(test_spsc_queue_push_pop_count);
    RUN_TEST(test_spsc_queue_null_params_and_capacity);
    RUN_TEST(test_spsc_queue_full_overflow_and_wraparound);
    RUN_TEST(test_spsc_queue_peek_and_clear);
    RUN_TEST(test_r5_spsc_queue_pthread_stress);
}
