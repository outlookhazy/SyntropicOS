/**
 * @file test_mailbox.c
 * @brief Unity tests for syn_mailbox.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_mailbox.h"

typedef struct {
    uint16_t id;
    int32_t  value;
} TestMsg;

static void test_mailbox(void)
{

    /* Static definition */
    SYN_MAILBOX_DEFINE(mbox, TestMsg, 4);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mbox));
    TEST_ASSERT_EQUAL_INT(0, syn_mailbox_pending(&mbox));
    TEST_ASSERT_EQUAL_INT(3, syn_mailbox_free(&mbox));

    /* Post messages */
    TestMsg m1 = { .id = 1, .value = 100 };
    TestMsg m2 = { .id = 2, .value = 200 };
    TestMsg m3 = { .id = 3, .value = 300 };

    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m1));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m2));
    TEST_ASSERT_TRUE(syn_mailbox_post(&mbox, &m3));
    TEST_ASSERT_FALSE(syn_mailbox_post(&mbox, &m1));
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_overflows(&mbox));

    TEST_ASSERT_EQUAL_INT(3, syn_mailbox_pending(&mbox));
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

    TEST_ASSERT_FALSE(syn_mailbox_receive(&mbox, &rx));
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));

    /* Runtime init */
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb2;
    syn_mailbox_init(&mb2, buf, sizeof(TestMsg), 4);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));

    TestMsg m = { .id = 99, .value = -1 };
    syn_mailbox_post(&mb2, &m);
    TEST_ASSERT_EQUAL_INT(1, syn_mailbox_pending(&mb2));

    /* Flush */
    syn_mailbox_flush(&mb2);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));
}

void run_mailbox_tests(void)
{
    RUN_TEST(test_mailbox);
}
