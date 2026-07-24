/**
 * @file test_metrics.c
 * @brief Unit tests for syn_metrics.
 */

#ifndef SYN_USE_ROUTER
#define SYN_USE_ROUTER 1
#endif
#include "unity/unity.h"
#include "syntropic/util/syn_metrics.h"
#include "syntropic/net/syn_router.h"
#include <string.h>

#if SYN_USE_METRICS

SYN_METRIC_DECLARE(test_counter, "test_counter", "Description", SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(test_gauge,   "test_gauge",   "Description", SYN_METRIC_TYPE_GAUGE);

void test_metric_counter_inc(void)
{
    uint64_t initial = test_counter.val.counter;
    SYN_METRIC_INC(test_counter);
    TEST_ASSERT_EQUAL_UINT64(initial + 1, test_counter.val.counter);
    
    SYN_METRIC_ADD(test_counter, 10);
    TEST_ASSERT_EQUAL_UINT64(initial + 11, test_counter.val.counter);
}

void test_metric_gauge_ops(void)
{
    SYN_METRIC_SET(test_gauge, 42);
    TEST_ASSERT_EQUAL_INT32(42, test_gauge.val.gauge);
    
    SYN_METRIC_INC(test_gauge);
    TEST_ASSERT_EQUAL_INT32(43, test_gauge.val.gauge);
    
    SYN_METRIC_DEC(test_gauge);
    TEST_ASSERT_EQUAL_INT32(42, test_gauge.val.gauge);
    
    SYN_METRIC_SUB(test_gauge, 2);
    TEST_ASSERT_EQUAL_INT32(40, test_gauge.val.gauge);
}

static int g_found_count = 0;
static void metric_visitor(const SYN_Metric *m, void *ctx)
{
    (void)ctx;
    if (strcmp(m->name, "test_counter") == 0 || strcmp(m->name, "test_gauge") == 0) {
        g_found_count++;
    }
}

void test_metric_registry(void)
{
    g_found_count = 0;
    syn_metrics_foreach(metric_visitor, NULL);
    TEST_ASSERT_EQUAL(2, g_found_count);
}

void test_metric_extra_coverage(void)
{
    syn_metrics_init();
    
    /* Double registration check */
    syn_metric_register(&test_counter);
    syn_metric_register(NULL);
    
    /* Foreach NULL callback check */
    syn_metrics_foreach(NULL, NULL);

#if SYN_USE_ROUTER
    SYN_Router router;
    SYN_RouterHandler handlers[2];
    SYN_Transport transport;
    memset(&transport, 0, sizeof(transport));
    syn_router_init(&router, 0x02, &transport, handlers, 2);
    syn_metrics_set_router(&router);
    
    syn_metrics_record("temp", 23.45f);
    syn_metrics_record("this_is_a_very_long_metric_name_exceeding_thirty_two_chars", 1.0f);
    syn_metrics_count("requests", 5);
#endif
}

#else

void test_metric_counter_inc(void) { TEST_IGNORE_MESSAGE("SYN_USE_METRICS disabled"); }
void test_metric_gauge_ops(void)   { TEST_IGNORE_MESSAGE("SYN_USE_METRICS disabled"); }
void test_metric_registry(void)    { TEST_IGNORE_MESSAGE("SYN_USE_METRICS disabled"); }
void test_metric_extra_coverage(void) { TEST_IGNORE_MESSAGE("SYN_USE_METRICS disabled"); }

#endif

void run_metrics_tests(void)
{
    SYN_METRIC_REGISTER(test_counter);
    SYN_METRIC_REGISTER(test_gauge);

    RUN_TEST(test_metric_counter_inc);
    RUN_TEST(test_metric_gauge_ops);
    RUN_TEST(test_metric_registry);
    RUN_TEST(test_metric_extra_coverage);
}
