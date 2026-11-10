/**
 * @file test_ao.c
 * @brief Unity tests for Active Object concurrency framework.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

enum { AO_ST_OFF, AO_ST_ON };
enum {
    AO_EV_TOGGLE,
    AO_EV_SET_VAL,
    AO_EV_SUSPEND,
    AO_EV_RESTART,
    AO_EV_SELF_POST,
    AO_EV_BOUNDED_SELF_POST,
    AO_EV_ARM_ISR_RACE
};

static int ao_action_called = 0;
static int ao_last_val = 0;

static void ao_on_toggle(void *ctx)
{
    (void)ctx;
    ao_action_called++;
}

static void ao_on_set_val(void *ctx)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    if (ao && ao->last_event.data) {
        ao_last_val = *(int *)ao->last_event.data;
    }
}

static void ao_on_self_suspend(void *ctx)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    ao_action_called++;
    syn_task_suspend(ao->task);
}

static void ao_on_self_restart(void *ctx)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    ao_action_called++;
    syn_task_restart(ao->task);
}

static void ao_on_self_post(void *ctx)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    ao_action_called++;
    (void)syn_ao_post(ao, AO_EV_SELF_POST, NULL);
}

static SYN_ActiveObject *s_isr_race_ao = NULL;

static void ao_mock_isr_post_once(void)
{
    mock_critical_enter_cb = NULL;
    if (s_isr_race_ao != NULL) {
        (void)syn_ao_post(s_isr_race_ao, AO_EV_BOUNDED_SELF_POST, NULL);
    }
}

static void ao_on_bounded_self_post(void *ctx)
{
    SYN_ActiveObject *ao = (SYN_ActiveObject *)ctx;
    ao_action_called++;
    if (ao_action_called < 10) {
        (void)syn_ao_post(ao, AO_EV_BOUNDED_SELF_POST, NULL);
    }
}

static void ao_on_arm_isr_race(void *ctx)
{
    s_isr_race_ao = (SYN_ActiveObject *)ctx;
    ao_action_called++;
    mock_critical_enter_cb = ao_mock_isr_post_once;
}

static const SYN_FSM_Transition test_ao_table[] = {
    {AO_ST_OFF, AO_EV_TOGGLE, AO_ST_ON, NULL, ao_on_toggle},
    {AO_ST_ON, AO_EV_TOGGLE, AO_ST_OFF, NULL, ao_on_toggle},
    {AO_ST_OFF, AO_EV_SET_VAL, AO_ST_OFF, NULL, ao_on_set_val},
    {AO_ST_ON, AO_EV_SET_VAL, AO_ST_ON, NULL, ao_on_set_val},
    {AO_ST_OFF, AO_EV_SUSPEND, AO_ST_ON, NULL, ao_on_self_suspend},
    {AO_ST_OFF, AO_EV_RESTART, AO_ST_ON, NULL, ao_on_self_restart},
    {AO_ST_OFF, AO_EV_SELF_POST, AO_ST_OFF, NULL, ao_on_self_post},
    {AO_ST_OFF, AO_EV_BOUNDED_SELF_POST, AO_ST_OFF, NULL, ao_on_bounded_self_post},
    {AO_ST_OFF, AO_EV_ARM_ISR_RACE, AO_ST_OFF, NULL, ao_on_arm_isr_race},
    SYN_FSM_END};

static SYN_ActiveObject test_ao_obj;
static SYN_AO_Event mbox_buf[4];

static void test_active_object(void)
{
    SYN_Task ao_task;
    syn_ao_init(&test_ao_obj, &ao_task, "test_ao", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);

    TEST_ASSERT_EQUAL(AO_ST_OFF, syn_fsm_state(&test_ao_obj.fsm));

    /* Post toggle event */
    bool ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_TRUE(ok);

    /* Setup a minimal scheduler and run once */
    SYN_Sched sched;
    syn_sched_init(&sched, &ao_task, 1);

    ao_action_called = 0;
    syn_sched_run(&sched);

    TEST_ASSERT_EQUAL(AO_ST_ON, syn_fsm_state(&test_ao_obj.fsm));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);

    /* Post again */
    ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_TRUE(ok);

    syn_sched_run(&sched);

    TEST_ASSERT_EQUAL(AO_ST_OFF, syn_fsm_state(&test_ao_obj.fsm));
    TEST_ASSERT_EQUAL_INT(2, ao_action_called);
}

static void test_active_object_overflow(void)
{
    SYN_Task ao_task;
    /* Mailbox capacity is 4. Since SPSC mailbox reserves 1 slot as sentinel, max capacity is 3. */
    syn_ao_init(&test_ao_obj, &ao_task, "test_ao_overflow", SYN_PRIO_NORMAL, test_ao_table,
                AO_ST_OFF, mbox_buf, 4);

    bool ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_TRUE(ok);
    ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_TRUE(ok);
    ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_TRUE(ok);

    /* 4th post should fail because mailbox is full */
    ok = syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_EQUAL_UINT32(1, syn_mailbox_overflows(&test_ao_obj.mailbox));
}

static void test_active_object_payload(void)
{
    SYN_Task ao_task;
    syn_ao_init(&test_ao_obj, &ao_task, "test_ao_payload", SYN_PRIO_NORMAL, test_ao_table,
                AO_ST_OFF, mbox_buf, 4);

    int val = 42;
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_SET_VAL, &val));

    SYN_Sched sched;
    syn_sched_init(&sched, &ao_task, 1);
    syn_sched_run(&sched);

    TEST_ASSERT_EQUAL_INT(42, ao_last_val);
}

static void test_active_object_empty_poll(void)
{
    SYN_Task ao_task;
    syn_ao_init(&test_ao_obj, &ao_task, "test_ao_empty", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);

    SYN_Sched sched;
    syn_sched_init(&sched, &ao_task, 1);

    /* Poll empty mailbox — protothread blocks task until syn_ao_post wakes it */
    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&test_ao_obj.mailbox));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, ao_task.state);
}

static SYN_PT_Status ao_idle_peer_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}

static void test_active_object_multi_task_wakeup(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_ao_init(&test_ao_obj, &tasks[0], "ao_multi", SYN_PRIO_HIGH, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    syn_task_create(&tasks[1], "idle_peer", ao_idle_peer_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Empty mailbox -> tasks[0] enters SYN_TASK_BLOCKED directly in tasks[] */
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Posting to AO wakes tasks[0] in-place inside multi-task array */
    ao_action_called = 0;
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);

    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_EQUAL(AO_ST_ON, syn_fsm_state(&test_ao_obj.fsm));

    /* Defensive NULL checks on AO task entry */
    SYN_Task null_ud_task = {0};
    TEST_ASSERT_EQUAL(PT_EXITED, tasks[0].func(NULL, &tasks[0]));
    TEST_ASSERT_EQUAL(PT_EXITED, tasks[0].func(&tasks[0].pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, tasks[0].func(&tasks[0].pt, &null_ud_task));
}

static int ao_peer_runs = 0;

static SYN_PT_Status ao_peer_worker_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        ao_peer_runs++;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_active_object_burst_shares_cpu(void)
{
    /* Uncontended: 3 queued events drain in a single syn_sched_run() dispatch */
    SYN_Task sole_task;
    syn_ao_init(&test_ao_obj, &sole_task, "ao_burst", SYN_PRIO_CRITICAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));

    SYN_Sched sole_sched;
    syn_sched_init(&sole_sched, &sole_task, 1);
    ao_action_called = 0;
    syn_sched_run(&sole_sched);
    TEST_ASSERT_EQUAL_INT(3, ao_action_called);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&test_ao_obj.mailbox));

    /* Contended with equal-priority CRITICAL worker: AO processes 1 event, yields PT_MORE
     * via PT_CONTINUE_IF_NEEDED so peer interleaves instead of starving until mailbox empty! */
    SYN_Task tasks[2];
    SYN_Sched contended_sched;
    syn_ao_init(&test_ao_obj, &tasks[0], "ao_contended", SYN_PRIO_CRITICAL, test_ao_table,
                AO_ST_OFF, mbox_buf, 4);
    syn_task_create(&tasks[1], "crit_peer", ao_peer_worker_fn, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&contended_sched, tasks, 2);

    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));

    ao_action_called = 0;
    ao_peer_runs = 0;

    syn_sched_run(&contended_sched); /* Dispatch 1: AO processes 1st event, yields PT_MORE */
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_EQUAL_INT(0, ao_peer_runs);

    syn_sched_run(&contended_sched); /* Dispatch 2: equal-prio peer interleaves! */
    TEST_ASSERT_EQUAL_INT(1, ao_peer_runs);

    syn_sched_run(&contended_sched); /* Dispatch 3: AO processes 2nd event, yields PT_MORE */
    TEST_ASSERT_EQUAL_INT(2, ao_action_called);
}

/** R4-K07: Self-suspend and self-restart inside AO event handlers preserve state & queued msgs. */
static void test_r4_active_object_self_suspend_and_restart(void)
{
    SYN_Task ao_task;
    SYN_Sched sched;

    /* 1. Self-suspend with remaining queued events */
    syn_ao_init(&test_ao_obj, &ao_task, "ao_susp", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    syn_sched_init(&sched, &ao_task, 1);

    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_SUSPEND, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));

    ao_action_called = 0;
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, ao_task.state);
    TEST_ASSERT_FALSE(syn_mailbox_empty(&test_ao_obj.mailbox));

    /* Remains suspended across subsequent scheduler ticks */
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);

    /* Resume wakes AO to drain the 2 remaining queued toggle events */
    syn_task_resume(&ao_task);
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(3, ao_action_called);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&test_ao_obj.mailbox));
    TEST_ASSERT_EQUAL(AO_ST_ON, syn_fsm_state(&test_ao_obj.fsm));

    /* 2. Self-restart inside handler (after blocking yield so pt.lc != 0) keeps PT_IS_IDLE(pt) */
    syn_ao_init(&test_ao_obj, &ao_task, "ao_rst", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    syn_sched_init(&sched, &ao_task, 1);

    /* First run blocks on empty mailbox -> sets non-zero pt.lc */
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_FALSE(PT_IS_IDLE(&ao_task.pt));

    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_RESTART, NULL));
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_TOGGLE, NULL));

    ao_action_called = 0;
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_TRUE(PT_IS_IDLE(&ao_task.pt));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, ao_task.state);
    TEST_ASSERT_FALSE(syn_mailbox_empty(&test_ao_obj.mailbox));

    /* Next dispatch runs cleanly from PT_BEGIN and consumes queued AO_EV_TOGGLE */
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(2, ao_action_called);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&test_ao_obj.mailbox));
    TEST_ASSERT_EQUAL(AO_ST_OFF, syn_fsm_state(&test_ao_obj.fsm));
}

static void test_active_object_self_post_burst_cap(void)
{
    SYN_Task ao_task;
    SYN_Sched sched;

    syn_ao_init(&test_ao_obj, &ao_task, "ao_self_post", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    syn_sched_init(&sched, &ao_task, 1U);

    /* 1 initial event queued -> single syn_sched_run() executes initial budget (1) and yields */
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_SELF_POST, NULL));
    ao_action_called = 0;
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_EQUAL_size_t(1U, syn_mailbox_pending(&test_ao_obj.mailbox));
    TEST_ASSERT_TRUE((ao_task.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);

    /* Post a 2nd event so initial budget is 2 -> single syn_sched_run() executes 2 and yields */
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_SELF_POST, NULL));
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(3, ao_action_called);
    TEST_ASSERT_EQUAL_size_t(2U, syn_mailbox_pending(&test_ao_obj.mailbox));
}

static void test_active_object_isr_post_at_zero_budget_no_underflow(void)
{
    SYN_Task ao_task;
    SYN_Sched sched;

    syn_ao_init(&test_ao_obj, &ao_task, "ao_isr_race", SYN_PRIO_NORMAL, test_ao_table, AO_ST_OFF,
                mbox_buf, 4);
    syn_sched_init(&sched, &ao_task, 1U);

    /* Queue 1 event (burst_budget = 1); its handler arms an ISR post inside PT_BLOCK_CONDITION */
    TEST_ASSERT_TRUE(syn_ao_post(&test_ao_obj, AO_EV_ARM_ISR_RACE, NULL));
    ao_action_called = 0;
    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    mock_critical_enter_cb = NULL;
    s_isr_race_ao = NULL;

    /* Must yield after initial budget (1) instead of underflowing burst_budget 0 -> SIZE_MAX */
    TEST_ASSERT_EQUAL_INT(1, ao_action_called);
    TEST_ASSERT_EQUAL_size_t(1U, syn_mailbox_pending(&test_ao_obj.mailbox));
    TEST_ASSERT_TRUE((ao_task.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
}

void run_ao_tests(void)
{
    RUN_TEST(test_active_object);
    RUN_TEST(test_active_object_overflow);
    RUN_TEST(test_active_object_payload);
    RUN_TEST(test_active_object_empty_poll);
    RUN_TEST(test_active_object_multi_task_wakeup);
    RUN_TEST(test_active_object_burst_shares_cpu);
    RUN_TEST(test_r4_active_object_self_suspend_and_restart);
    RUN_TEST(test_active_object_self_post_burst_cap);
    RUN_TEST(test_active_object_isr_post_at_zero_budget_no_underflow);
}
