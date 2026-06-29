/**
 * @file test_watchdog.c
 * @brief Unity tests for syn_watchdog.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_watchdog.h"

static int wdt_timeout_count = 0;
static const char *wdt_timeout_name = NULL;

static void wdt_on_timeout(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry,
                           void *ctx)
{
    (void)wdt; (void)ctx;
    wdt_timeout_count++;
    wdt_timeout_name = entry->name;
}

static void test_watchdog(void)
{

    mock_tick_ms = 0;
    wdt_timeout_count = 0;

    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[4];
    syn_watchdog_init(&wdt, entries, 4, wdt_on_timeout, NULL);

    int8_t id0 = syn_watchdog_register(&wdt, "task_a", 100);
    int8_t id1 = syn_watchdog_register(&wdt, "task_b", 200);
    TEST_ASSERT_TRUE(id0 >= 0);
    TEST_ASSERT_TRUE(id1 >= 0);

    /* No timeout yet */
    mock_tick_advance(50);
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    /* Check in task_a */
    syn_watchdog_checkin(&wdt, id0);

    /* task_a deadline at 150ms, task_b at 200ms */
    mock_tick_advance(60);  /* now 110ms */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    mock_tick_advance(100); /* now 210ms — task_b should time out */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
    TEST_ASSERT_EQUAL_INT(0, strcmp(wdt_timeout_name, "task_b"));

    /* Unregister */
    syn_watchdog_unregister(&wdt, id1);
    wdt_timeout_count = 0;
    mock_tick_advance(500);
    syn_watchdog_update(&wdt);
    /* Only id0 should fire (id1 unregistered) */
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
}

void run_watchdog_tests(void)
{
    RUN_TEST(test_watchdog);
}
