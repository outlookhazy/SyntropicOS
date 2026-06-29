/**
 * @file test_workqueue.c
 * @brief Unity tests for syn_workqueue.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_workqueue.h"

static int wq_sum = 0;
static void wq_handler(void *ctx) { wq_sum += *(int *)ctx; }

static void test_workqueue(void)
{

    wq_sum = 0;

    SYN_WorkItem items[4];
    SYN_WorkQueue wq;
    syn_workqueue_init(&wq, items, 4);

    TEST_ASSERT_TRUE(syn_workqueue_empty(&wq));
    TEST_ASSERT_EQUAL_INT(0, syn_workqueue_pending(&wq));

    static int v1 = 10, v2 = 20, v3 = 30;

    TEST_ASSERT_TRUE(syn_workqueue_post(&wq, wq_handler, &v1));
    TEST_ASSERT_TRUE(syn_workqueue_post(&wq, wq_handler, &v2));
    TEST_ASSERT_TRUE(syn_workqueue_post(&wq, wq_handler, &v3));
    /* Queue capacity = 4, but SPSC ring uses one slot as sentinel,
       so max usable = 3 */
    TEST_ASSERT_FALSE(syn_workqueue_post(&wq, wq_handler, &v1));
    TEST_ASSERT_EQUAL_INT(1, syn_workqueue_overflows(&wq));

    TEST_ASSERT_FALSE(syn_workqueue_empty(&wq));
    TEST_ASSERT_EQUAL_INT(3, syn_workqueue_pending(&wq));

    size_t processed = syn_workqueue_process(&wq);
    TEST_ASSERT_EQUAL_INT(3, processed);
    TEST_ASSERT_EQUAL_INT(60, wq_sum);
    TEST_ASSERT_TRUE(syn_workqueue_empty(&wq));

    /* Process empty queue = 0 */
    processed = syn_workqueue_process(&wq);
    TEST_ASSERT_EQUAL_INT(0, processed);
}

void run_workqueue_tests(void)
{
    RUN_TEST(test_workqueue);
}
