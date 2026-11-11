/**
 * @file test_mailbox.c
 * @brief Unity tests for syn_mailbox.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_mailbox.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

typedef struct {
    uint16_t id;
    int32_t value;
} TestMsg;

static void test_mailbox(void)
{
    /* Static definition with count = 4 -> can hold 4 messages */
    SYN_MAILBOX_DEFINE(mbox, TestMsg, 4);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mbox));
    TEST_ASSERT_EQUAL_INT(0, syn_mailbox_pending(&mbox));
    TEST_ASSERT_EQUAL_INT(4, syn_mailbox_free(&mbox));

    /* Post 4 messages */
    TestMsg m1 = {.id = 1, .value = 100};
    TestMsg m2 = {.id = 2, .value = 200};
    TestMsg m3 = {.id = 3, .value = 300};
    TestMsg m4 = {.id = 4, .value = 400};

    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m1));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m2));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m3));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m4));
    TEST_ASSERT_FALSE(syn_mailbox_post(&mbox, &m1));
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_overflows(&mbox));

    TEST_ASSERT_EQUAL_INT(4, syn_mailbox_pending(&mbox));
    TEST_ASSERT_TRUE(syn_mailbox_full(&mbox));

    /* Peek */
    const TestMsg *peek = (const TestMsg *)syn_mailbox_peek(&mbox);
    TEST_ASSERT_TRUE(peek != NULL);
    TEST_ASSERT_EQUAL_INT(1, peek->id);

    /* Receive */
    TestMsg rx;
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_EQUAL_INT(1, rx.id);
    TEST_ASSERT_EQUAL_INT(100, rx.value);

    TEST_ASSERT_TRUE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_EQUAL_INT(2, rx.id);
    TEST_ASSERT_EQUAL_INT(200, rx.value);

    TEST_ASSERT_TRUE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_EQUAL_INT(3, rx.id);
    TEST_ASSERT_EQUAL_INT(300, rx.value);

    TEST_ASSERT_TRUE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_EQUAL_INT(4, rx.id);
    TEST_ASSERT_EQUAL_INT(400, rx.value);

    TEST_ASSERT_FALSE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));

    /* Runtime init */
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb2;
    syn_mailbox_init(&mb2, buf, sizeof(TestMsg), 4);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));

    TestMsg m = {.id = 99, .value = -1};
    syn_mailbox_post(&mb2, &m);
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_pending(&mb2));

    /* Flush */
    syn_mailbox_flush(&mb2);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));
}

static void test_mailbox_capacity_one(void)
{
    SYN_MAILBOX_DEFINE(mb1, TestMsg, 1);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb1));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mb1));
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_free(&mb1));

    TestMsg m = {.id = 42, .value = 1337};
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb1, &m));
    TEST_ASSERT_TRUE(syn_mailbox_full(&mb1));
    TEST_ASSERT_EQUAL_INT(0, syn_mailbox_free(&mb1));
    TEST_ASSERT_FALSE(syn_mailbox_post(&mb1, &m));
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_overflows(&mb1));

    TestMsg rx;
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb1, &rx));
    TEST_ASSERT_EQUAL_UINT16(42, rx.id);
    TEST_ASSERT_EQUAL_INT32(1337, rx.value);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb1));
}

static void test_mailbox_wraparound_and_notify(void)
{
    uint8_t buf[3 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 3); /* capacity = 3 (2 usable) */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE
    syn_mailbox_set_notify(&mb, true);
    TEST_ASSERT_TRUE(mb.notify);
#endif

    TestMsg m1 = {.id = 10, .value = 1000};
    TestMsg m2 = {.id = 20, .value = 2000};
    TestMsg m3 = {.id = 30, .value = 3000};
    TestMsg rx;

    /* Push 2, pop 1, push 1 (wraps head back to index 0) */
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &m1));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &m2));
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL_UINT16(10, rx.id);

    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &m3));
    TEST_ASSERT_TRUE(syn_mailbox_full(&mb));

    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL_UINT16(20, rx.id);
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL_UINT16(30, rx.id);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
}

static void test_mailbox_zero_and_single_slot_raw_capacity_guards(void)
{
    uint8_t buf[sizeof(TestMsg)];
    TestMsg m = {.id = 9, .value = 99};
    TestMsg rx;

    for (size_t cap = 0U; cap <= 1U; cap++) {
        SYN_Mailbox mb;
        syn_mailbox_init(&mb, buf, sizeof(TestMsg), cap);

        TEST_ASSERT_EQUAL_size_t(0U, syn_mailbox_free(&mb));
        TEST_ASSERT_EQUAL_size_t(0U, syn_mailbox_pending(&mb));
        TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
        TEST_ASSERT_TRUE(syn_mailbox_full(&mb));
        TEST_ASSERT_NULL(syn_mailbox_peek(&mb));
        TEST_ASSERT_FALSE(syn_mailbox_post(&mb, &m));
        TEST_ASSERT_EQUAL_UINT32(1U, syn_mailbox_overflows(&mb));
        TEST_ASSERT_FALSE(syn_mailbox_receive(&mb, &rx));
    }

    /* Zero msg_size, NULL buf, or NULL msg rejected safely without memcpy UB */
    SYN_Mailbox zero_sz_mb;
    syn_mailbox_init(&zero_sz_mb, buf, 0U, 4U);
    TEST_ASSERT_FALSE(syn_mailbox_post(&zero_sz_mb, &m));
    zero_sz_mb.head = 1U;
    TEST_ASSERT_NULL(syn_mailbox_peek(&zero_sz_mb));
    TEST_ASSERT_FALSE(syn_mailbox_receive(&zero_sz_mb, &rx));

    SYN_Mailbox null_buf_mb;
    syn_mailbox_init(&null_buf_mb, NULL, sizeof(TestMsg), 4U);
    TEST_ASSERT_FALSE(syn_mailbox_post(&null_buf_mb, &m));
    null_buf_mb.head = 1U;
    TEST_ASSERT_NULL(syn_mailbox_peek(&null_buf_mb));
    TEST_ASSERT_FALSE(syn_mailbox_receive(&null_buf_mb, &rx));

    SYN_Mailbox valid_mb;
    uint8_t valid_buf[4U * sizeof(TestMsg)];
    syn_mailbox_init(&valid_mb, valid_buf, sizeof(TestMsg), 4U);
    TEST_ASSERT_FALSE(syn_mailbox_post(&valid_mb, NULL));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_mailbox_overflows(&valid_mb));
    TEST_ASSERT_TRUE(syn_mailbox_post(&valid_mb, &m));
    TEST_ASSERT_FALSE(syn_mailbox_receive(&valid_mb, NULL));
    TEST_ASSERT_TRUE(syn_mailbox_receive(&valid_mb, &rx));

    /* Oversized capacity multiplication overflow guard */
    SYN_Mailbox over_mb;
    syn_mailbox_init(&over_mb, valid_buf, sizeof(TestMsg), (SIZE_MAX / sizeof(TestMsg)) + 1U);
    TEST_ASSERT_NULL(over_mb.buf);
    TEST_ASSERT_EQUAL_size_t(0U, over_mb.capacity);
    TEST_ASSERT_FALSE(syn_mailbox_post(&over_mb, &m));
    TEST_ASSERT_EQUAL_UINT32(0U, syn_mailbox_overflows(&over_mb));
    TEST_ASSERT_FALSE(syn_mailbox_receive(&over_mb, &rx));
    TEST_ASSERT_NULL(syn_mailbox_peek(&over_mb));

    /* Corrupted post-init capacity > SIZE_MAX / msg_size rejected in post/receive/peek */
    valid_mb.capacity = (SIZE_MAX / sizeof(TestMsg)) + 1U;
    valid_mb.head = 1U;
    valid_mb.tail = 0U;
    TEST_ASSERT_FALSE(syn_mailbox_post(&valid_mb, &m));
    TEST_ASSERT_FALSE(syn_mailbox_receive(&valid_mb, &rx));
    TEST_ASSERT_NULL(syn_mailbox_peek(&valid_mb));
}

static void test_mailbox_define_alignment_after_odd_byte(void)
{
    static uint8_t s_odd_pad;
    SYN_MAILBOX_DEFINE(s_aligned_mbox, uint32_t, 1);

    s_odd_pad = 0x5AU;
    (void)s_odd_pad;
    s_aligned_mbox.head = 0U;
    s_aligned_mbox.tail = 0U;
    s_aligned_mbox.overflow = 0U;

    /* Declared element type must match message type for alignment & C99 strict aliasing */
    TEST_ASSERT_EQUAL_size_t(sizeof(uint32_t), sizeof(s_aligned_mbox_buf[0]));

    uint32_t tx = 0xDEADBEEFU;
    TEST_ASSERT_TRUE(syn_mailbox_post(&s_aligned_mbox, &tx));

    const void *raw = syn_mailbox_peek(&s_aligned_mbox);
    TEST_ASSERT_NOT_NULL(raw);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)((uintptr_t)raw % sizeof(uint32_t)));

    const uint32_t *peek = (const uint32_t *)raw;
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEFU, *peek);
}

void run_mailbox_tests(void)
{
    RUN_TEST(test_mailbox);
    RUN_TEST(test_mailbox_capacity_one);
    RUN_TEST(test_mailbox_wraparound_and_notify);
    RUN_TEST(test_mailbox_zero_and_single_slot_raw_capacity_guards);
    RUN_TEST(test_mailbox_define_alignment_after_odd_byte);
}
