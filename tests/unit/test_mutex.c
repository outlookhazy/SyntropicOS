/**
 * @file test_mutex.c
 * @brief Unity tests for priority-inheriting mutex (syn_mutex).
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_mutex.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static SYN_Mutex g_test_mutex;
static int g_exec_order[8];
static int g_exec_count = 0;

static SYN_PT_Status dummy_pt_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    PT_END(pt);
}

static void test_mutex_init_and_query(void)
{
    SYN_Mutex m;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mutex_init(NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&m));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));
    TEST_ASSERT_NULL(syn_mutex_get_owner(&m));

    TEST_ASSERT_FALSE(syn_mutex_is_locked(NULL));
    TEST_ASSERT_NULL(syn_mutex_get_owner(NULL));
}

static void test_mutex_basic_trylock_and_unlock(void)
{
    SYN_Mutex m;
    SYN_Task t1, t2;

    syn_task_create(&t1, "t1", dummy_pt_fn, 1, NULL);
    syn_task_create(&t2, "t2", dummy_pt_fn, 2, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&m));

    /* NULL parameters */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mutex_try_lock(NULL, &t1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mutex_unlock(NULL, &t1));

    /* Acquire by t1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &t1));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));
    TEST_ASSERT_EQUAL_PTR(&t1, syn_mutex_get_owner(&m));

    /* Attempt by t2 should fail (BUSY) */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &t2));

    /* Unlock by wrong task should fail */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mutex_unlock(&m, &t2));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));

    /* Unlock by owner succeeds */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &t1));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));
    TEST_ASSERT_NULL(syn_mutex_get_owner(&m));

    /* Unlocking when already unlocked returns error */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mutex_unlock(&m, &t1));
}

static void test_mutex_standalone_lock_unlock(void)
{
    SYN_Mutex m;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&m));

    /* Standalone lock with task == NULL */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, NULL));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));
    TEST_ASSERT_NULL(syn_mutex_get_owner(&m));

    /* Standalone unlock */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, NULL));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));
}

static void test_mutex_recursive_locking(void)
{
    SYN_Mutex m;
    SYN_Task t;

    syn_task_create(&t, "rec_task", dummy_pt_fn, 0, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&m));

    /* Acquire 3 times */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &t));
    TEST_ASSERT_EQUAL_UINT16(1, m.lock_count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &t));
    TEST_ASSERT_EQUAL_UINT16(2, m.lock_count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &t));
    TEST_ASSERT_EQUAL_UINT16(3, m.lock_count);

    /* Release depth 1 and 2: still locked */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &t));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));
    TEST_ASSERT_EQUAL_UINT16(2, m.lock_count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &t));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));
    TEST_ASSERT_EQUAL_UINT16(1, m.lock_count);

    /* Release final depth: unlocked */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &t));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));
    TEST_ASSERT_EQUAL_UINT16(0, m.lock_count);
    TEST_ASSERT_NULL(syn_mutex_get_owner(&m));
}

static void test_mutex_priority_inheritance(void)
{
    SYN_Mutex m;
    SYN_Task task_l, task_m, task_h;

    syn_task_create(&task_l, "low", dummy_pt_fn, 3, NULL);
    syn_task_create(&task_m, "med", dummy_pt_fn, 1, NULL);
    syn_task_create(&task_h, "high", dummy_pt_fn, 0, NULL);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&m));

    /* Task L acquires mutex */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &task_l));
    TEST_ASSERT_EQUAL_UINT8(3, task_l.priority);

    /* Task M (prio 1) attempts acquisition — fails and boosts Task L from 3 to 1 */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &task_m));
    TEST_ASSERT_EQUAL_UINT8(1, task_l.priority); /* Boosted to 1 by Task M */

    /* Task H (prio 0) attempts acquisition — fails and boosts Task L to 0 */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &task_h));
    TEST_ASSERT_EQUAL_UINT8(0, task_l.priority); /* Boosted to 0 by Task H */

    /* Duplicate try_lock keeps task_h and task_m in waiters[] without duplicating task_m */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &task_m));
    TEST_ASSERT_EQUAL_PTR(&task_m, m.waiters[0]);
    TEST_ASSERT_EQUAL_PTR(&task_h, m.waiters[1]);
    TEST_ASSERT_NULL(m.waiters[2]);

    /* Task L unlocks mutex — priority restored to base priority 3 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &task_l));
    TEST_ASSERT_EQUAL_UINT8(3, task_l.priority);
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));

    /* Task H now successfully acquires */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &task_h));
    TEST_ASSERT_EQUAL_PTR(&task_h, syn_mutex_get_owner(&m));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &task_h));
}

/* ── Cooperative Multitasking Scheduler Integration Tests ──────────────── */

static volatile int g_low_has_lock = 0;

static SYN_PT_Status pt_task_low_inversion(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    /* Low acquires mutex immediately */
    PT_MUTEX_LOCK(pt, task, &g_test_mutex);
    g_exec_order[g_exec_count++] = 10;
    g_low_has_lock = 1;
    PT_PASS(pt);

    /* After pass, High will have tried to lock and boosted Low to prio 0 */
    TEST_ASSERT_EQUAL_UINT8(0, task->priority);
    g_exec_order[g_exec_count++] = 11;

    PT_MUTEX_UNLOCK(&g_test_mutex, task);
    TEST_ASSERT_EQUAL_UINT8(3, task->priority); /* Restored */
    g_exec_order[g_exec_count++] = 12;

    PT_END(pt);
}

static SYN_PT_Status pt_task_med(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    /* Pass on initial round to allow Low to acquire mutex */
    PT_PASS(pt);

    /* Medium-priority task doing work */
    g_exec_order[g_exec_count++] = 20;
    PT_CONTINUE(pt);
    g_exec_order[g_exec_count++] = 21;

    PT_END(pt);
}

static SYN_PT_Status pt_task_high_inversion(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    /* High waits for Low to acquire first */
    PT_WAIT_UNTIL(pt, g_low_has_lock == 1);

    /* Now High attempts to lock — gets blocked and boosts Low */
    PT_MUTEX_LOCK(pt, task, &g_test_mutex);
    g_exec_order[g_exec_count++] = 30;

    PT_MUTEX_UNLOCK(&g_test_mutex, task);
    g_exec_order[g_exec_count++] = 31;

    PT_END(pt);
}

static void test_mutex_priority_inversion_prevention(void)
{
    SYN_Sched sched;
    SYN_Task tasks[3];

    g_exec_count = 0;
    g_low_has_lock = 0;
    PT_MUTEX_INIT(&g_test_mutex);

    syn_task_create(&tasks[0], "high", pt_task_high_inversion, 0, NULL);
    syn_task_create(&tasks[1], "med", pt_task_med, 1, NULL);
    syn_task_create(&tasks[2], "low", pt_task_low_inversion, 3, NULL);

    syn_sched_init(&sched, tasks, 3);

    /* Run scheduler until all tasks finish */
    int safety = 0;
    while (syn_sched_run(&sched) && safety++ < 20)
        ;

    /* Verify order:
     * 1. Low locks mutex (10).
     * 2. High attempts to lock, gets blocked (WAITING) and boosts Low to prio 0.
     * 3. Scheduler re-scans: Low is prio 0, Med is prio 1. Low RUNS before Med (11, 12)!
     * 4. High acquires mutex, runs (30, 31).
     * 5. Med runs (20, 21).
     */
    TEST_ASSERT_EQUAL_INT(7, g_exec_count);
    TEST_ASSERT_EQUAL_INT(10, g_exec_order[0]); /* Low lock */
    TEST_ASSERT_EQUAL_INT(11, g_exec_order[1]); /* Low boosted run before Med */
    TEST_ASSERT_EQUAL_INT(12, g_exec_order[2]); /* Low unlock & restore */
    TEST_ASSERT_EQUAL_INT(30, g_exec_order[3]); /* High acquire */
    TEST_ASSERT_EQUAL_INT(31, g_exec_order[4]); /* High unlock */
    TEST_ASSERT_EQUAL_INT(20, g_exec_order[5]); /* Med runs part 1 */
    TEST_ASSERT_EQUAL_INT(21, g_exec_order[6]); /* Med runs part 2 */
}

static void test_mutex_trylock_macro(void)
{
    SYN_Mutex m;
    SYN_Task t;

    syn_task_create(&t, "test", dummy_pt_fn, 0, NULL);
    PT_MUTEX_INIT(&m);

    TEST_ASSERT_TRUE(PT_MUTEX_TRYLOCK(&m, &t));
    TEST_ASSERT_TRUE(syn_mutex_is_locked(&m));
    TEST_ASSERT_EQUAL(SYN_OK, PT_MUTEX_UNLOCK(&m, &t));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m));
}

static uint32_t g_low_holder_unlock_ms = UINT32_MAX;

static SYN_PT_Status pt_crit_flood_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    while (g_low_holder_unlock_ms == UINT32_MAX) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status pt_high_contender_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, g_low_has_lock == 1);
    PT_MUTEX_LOCK(pt, task, &g_test_mutex);
    PT_MUTEX_UNLOCK(&g_test_mutex, task);
    PT_END(pt);
}

static SYN_PT_Status pt_low_holder_deadline_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, task, &g_test_mutex);
    g_low_has_lock = 1;
    PT_PASS(pt);
    g_low_holder_unlock_ms = mock_tick_ms;
    PT_MUTEX_UNLOCK(&g_test_mutex, task);
    PT_END(pt);
}

static void test_mutex_boosted_deadline_and_suspended_waiter(void)
{
    /* Part 1: 4-tier starvation bound under CRITICAL PT_MORE flood.
     * Low (50 ms window) acquires mutex at t=0, High (5 ms window) contends at t=0,
     * Critical floods PT_MORE continuously. Boosted Low must preempt Critical via Overdue
     * at t=5 ms (High's window), NOT t=50 ms (Low's unboosted window)! */
    mock_tick_ms = 0;
    g_low_has_lock = 0;
    g_low_holder_unlock_ms = UINT32_MAX;
    PT_MUTEX_INIT(&g_test_mutex);

    SYN_Sched sched;
    SYN_Task tasks[3];
    syn_task_create(&tasks[0], "crit_flood", pt_crit_flood_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&tasks[1], "high_wait", pt_high_contender_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[2], "low_hold", pt_low_holder_deadline_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 3);

    /* t = 0: suspend crit_flood while low_hold locks (due=50) and high_wait contends,
     * boosting low_hold to HIGH and tightening due_ms from 50 down to 5! */
    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched); /* high_wait waits (g_low_has_lock == 0) */
    syn_sched_run(&sched); /* low_hold locks mutex & passes (due_ms = 50) */
    syn_sched_run(&sched); /* high_wait contends -> boosts low_hold (due_ms = 5) */
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[2].priority);
    TEST_ASSERT_EQUAL_UINT32(5U, tasks[2].due_ms);

    /* Resume CRITICAL PT_MORE flood at t = 0: boosted low_hold must preempt at t = 5 ms */
    syn_task_resume(&tasks[0]);
    for (uint32_t ms = 0U; ms <= 10U && g_low_holder_unlock_ms == UINT32_MAX; ms++) {
        mock_tick_ms = ms;
        syn_sched_run(&sched);
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_UINT32(5U, g_low_holder_unlock_ms);

    /* Part 2: Suspended waiter replacement on lock & flag preservation on unlock */
    SYN_Mutex m;
    SYN_Task owner, w_high, w_norm;
    syn_task_create(&owner, "owner", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&w_high, "w_high", dummy_pt_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&w_norm, "w_norm", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);
    PT_MUTEX_INIT(&m);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m, &owner));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &w_high));
    TEST_ASSERT_EQUAL_PTR(&w_high, m.waiters[0]);

    /* Suspend w_high: lower-priority READY contender w_norm prunes & replaces inactive slot */
    syn_task_suspend(&w_high);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m, &w_norm));
    TEST_ASSERT_EQUAL_PTR(&w_norm, m.waiters[0]);

    /* Suspend w_norm too before unlock: unlock must NOT corrupt SLEEPING flags with MORE */
    syn_task_suspend(&w_norm);
    TEST_ASSERT_EQUAL_UINT8(SYN_SCHED_FLAG_SLEEPING, w_norm.sched_flags);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m, &owner));
    TEST_ASSERT_EQUAL_UINT8(SYN_SCHED_FLAG_SLEEPING, w_norm.sched_flags);
}

static void test_mutex_multi_hold_unlink_and_saturation(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task task_low = {0};
    SYN_Task task_norm = {0};
    SYN_Task task_high = {0};

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    syn_task_create(&task_low, "low", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&task_norm, "norm", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&task_high, "high", dummy_pt_fn, SYN_PRIO_HIGH, NULL);

    /* Lock m1 then m2 so held_mutexes list is m2 -> m1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &task_low));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &task_low));
    TEST_ASSERT_EQUAL_PTR(&m2, task_low.held_mutexes);
    TEST_ASSERT_EQUAL_PTR(&m1, m2.next_held);

    /* Contend m1 with NORMAL and m2 with HIGH */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &task_norm));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, task_low.priority);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &task_high));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, task_low.priority);

    /* Unlock m1 first (non-head unlink traverses pp = &(*pp)->next_held): keeps HIGH from m2 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &task_low));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m1));
    TEST_ASSERT_EQUAL_PTR(&m2, task_low.held_mutexes);
    TEST_ASSERT_NULL(m2.next_held);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, task_low.priority);

    /* Unlock m2: held_mutexes becomes NULL and priority restores to LOW */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &task_low));
    TEST_ASSERT_NULL(task_low.held_mutexes);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, task_low.priority);

    /* Recursive lock saturation at UINT16_MAX and NULL task unlock/lock checks */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &task_low));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mutex_unlock(&m1, NULL));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, NULL));

    m1.lock_count = UINT16_MAX;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mutex_try_lock(&m1, &task_low));
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, m1.lock_count);

    m1.lock_count = 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &task_low));
}

typedef struct {
    SYN_Mutex *m1;
    SYN_Mutex *m2;
    bool use_pt_exit;
    int acquired_flag;
    int waiter_completions;
} MutexExitTestCtx;

static SYN_PT_Status pt_mutex_leak_holder_fn(SYN_PT *pt, SYN_Task *task)
{
    MutexExitTestCtx *ctx = (MutexExitTestCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, task, ctx->m1);
    /* Recursive second lock on m1 + additional held mutex m2 */
    (void)syn_mutex_try_lock(ctx->m1, task);
    if (ctx->m2 != NULL) {
        PT_MUTEX_LOCK(pt, task, ctx->m2);
    }
    ctx->acquired_flag = 1;
    PT_PASS(pt);
    if (ctx->use_pt_exit) {
        PT_EXIT(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status pt_mutex_exit_waiter_fn(SYN_PT *pt, SYN_Task *task)
{
    MutexExitTestCtx *ctx = (MutexExitTestCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, ctx->acquired_flag == 1);
    PT_MUTEX_LOCK(pt, task, ctx->m1);
    ctx->waiter_completions++;
    PT_MUTEX_UNLOCK(ctx->m1, task);
    PT_END(pt);
}

/** R3-K02: Task exiting via PT_EXIT/PT_END while holding mutexes wakes waiters. */
static void test_mutex_release_on_task_exit_and_end(void)
{
    for (int mode = 0; mode < 2; mode++) {
        SYN_Mutex m1;
        SYN_Mutex m2;
        SYN_Task tasks[2];
        SYN_Sched sched;
        MutexExitTestCtx ctx = {&m1, &m2, (mode == 0), 0, 0};

        PT_MUTEX_INIT(&m1);
        PT_MUTEX_INIT(&m2);
        mock_tick_ms = 0U;
        syn_task_create(&tasks[0], "waiter", pt_mutex_exit_waiter_fn, SYN_PRIO_HIGH, &ctx);
        syn_task_create(&tasks[1], "holder", pt_mutex_leak_holder_fn, SYN_PRIO_LOW, &ctx);
        syn_sched_init(&sched, tasks, 2);

        int safety = 0;
        while (syn_sched_run(&sched) && safety++ < 16) {
        }

        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, tasks[0].state);
        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, tasks[1].state);
        TEST_ASSERT_EQUAL_INT(1, ctx.waiter_completions);
        TEST_ASSERT_FALSE(syn_mutex_is_locked(&m1));
        TEST_ASSERT_FALSE(syn_mutex_is_locked(&m2));
        TEST_ASSERT_NULL(tasks[1].held_mutexes);
        TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);

        /* Re-creating into the exited slot must not inherit stale recursive ownership */
        syn_task_create(&tasks[1], "reused", dummy_pt_fn, SYN_PRIO_LOW, NULL);
        TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[0]));
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[1]));
        TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[0]));
    }
}

typedef struct {
    SYN_Mutex *mutex;
    int low_locked;
    int low_post_unlock_continued;
    int order[8];
    int count;
} R4MutexCtx;

static SYN_PT_Status pt_r4_low_unlock_share_fn(SYN_PT *pt, SYN_Task *task)
{
    R4MutexCtx *ctx = (R4MutexCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, task, ctx->mutex);
    ctx->low_locked = 1;
    PT_PASS(pt);
    ctx->order[ctx->count++] = 1;
    PT_MUTEX_UNLOCK(ctx->mutex, task);
    PT_CONTINUE_IF_NEEDED(pt, task);
    ctx->low_post_unlock_continued = 1;
    ctx->order[ctx->count++] = 4;
    PT_END(pt);
}

static SYN_PT_Status pt_r4_crit_waiter_fn(SYN_PT *pt, SYN_Task *task)
{
    R4MutexCtx *ctx = (R4MutexCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, ctx->low_locked == 1);
    PT_MUTEX_LOCK(pt, task, ctx->mutex);
    ctx->order[ctx->count++] = 2;
    PT_MUTEX_UNLOCK(ctx->mutex, task);
    PT_END(pt);
}

static SYN_PT_Status pt_r4_high_waiter_fn(SYN_PT *pt, SYN_Task *task)
{
    R4MutexCtx *ctx = (R4MutexCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, ctx->low_locked == 1);
    PT_MUTEX_LOCK(pt, task, ctx->mutex);
    ctx->order[ctx->count++] = 3;
    PT_MUTEX_UNLOCK(ctx->mutex, task);
    PT_END(pt);
}

static SYN_PT_Status pt_r4_low_flood_fn(SYN_PT *pt, SYN_Task *task)
{
    R4MutexCtx *ctx = (R4MutexCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, ctx->low_locked == 1);
    while (ctx->count < 4) {
        ctx->order[ctx->count++] = 9;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

/** R4-K03 & R4-K04(a,d): Unlock refreshes share state and wakes all contended waiters to WORK. */
static void test_r4_mutex_unlock_share_refresh_and_multi_waiter_wake(void)
{
    SYN_Mutex m;
    SYN_Task tasks[4];
    SYN_Sched sched;
    R4MutexCtx ctx = {&m, 0, 0, {0}, 0};

    PT_MUTEX_INIT(&m);
    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "crit_w", pt_r4_crit_waiter_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "high_w", pt_r4_high_waiter_fn, SYN_PRIO_HIGH, &ctx);
    syn_task_create(&tasks[2], "low_own", pt_r4_low_unlock_share_fn, SYN_PRIO_LOW, &ctx);
    syn_task_create(&tasks[3], "low_fld", pt_r4_low_flood_fn, SYN_PRIO_LOW, &ctx);
    syn_sched_init(&sched, tasks, 4);

    /* Suspend flood task while low_own acquires m and BOTH crit_w and high_w contend */
    syn_task_suspend(&tasks[3]);
    syn_sched_run(&sched); /* crit_w waits on low_locked */
    syn_sched_run(&sched); /* high_w waits on low_locked */
    syn_sched_run(&sched); /* low_own locks m, sets low_locked = 1, passes */
    syn_sched_run(&sched); /* crit_w contends on m -> boosts low_own to CRITICAL */
    /* Force high_w to contend before low_own resumes by temporarily suspending low_own */
    syn_task_suspend(&tasks[2]);
    syn_sched_run(&sched); /* high_w contends on m -> both crit_w and high_w in m.waiters[] */
    syn_task_resume(&tasks[2]);
    syn_task_resume(&tasks[3]);
    tasks[2].priority = SYN_PRIO_CRITICAL;

    /* Next dispatch runs low_own: unlocks m and hits PT_CONTINUE_IF_NEEDED -> yields immediately!
     */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(0, ctx.low_post_unlock_continued);
    TEST_ASSERT_EQUAL_INT(1, ctx.count);
    TEST_ASSERT_EQUAL_INT(1, ctx.order[0]);

    /* Both crit_w (2) and high_w (3) woke to WORK rank ahead of low_fld (9) and low_own (4)! */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, ctx.count);
    TEST_ASSERT_EQUAL_INT(2, ctx.order[1]);
    TEST_ASSERT_EQUAL_INT(3, ctx.order[2]);
}

/** R4-K04(b,c,e): Manual boost preservation, stale waiter cleanup, and full waiters[] array. */
static void test_r4_mutex_manual_boost_stale_waiter_and_full_array(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task owner;
    SYN_Task w_crit;
    SYN_Task w_high;
    SYN_Task w_norm;

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    syn_task_create(&owner, "owner", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&w_crit, "w_crit", dummy_pt_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&w_high, "w_high", dummy_pt_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&w_norm, "w_norm", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);

    /* (b) Manual boost to CRITICAL on LOW owner is NOT demoted when NORMAL fails try_lock */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    syn_task_boost_priority(&owner, SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &w_norm));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);
    /* Restoring manual boost drops only to active mutex waiter w_norm (NORMAL) */
    syn_task_restore_priority(&owner);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, owner.priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &owner));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, owner.priority);

    /* (c) Standalone try_lock + restart (unbound) drops stale boost and keeps HIGH waiter wakeup */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &w_crit));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);
    syn_task_restart(&w_crit);
    TEST_ASSERT_EQUAL_UINT8(0U, w_crit.sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    w_high.sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &w_high));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, owner.priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &owner));
    TEST_ASSERT_TRUE((w_high.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);

    /* Delayed task passed directly to try_lock is ignored as inactive waiter */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    w_crit.delay_until = 50U;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &w_crit));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, owner.priority);
    w_crit.delay_until = 0U;
    w_crit.sched_flags = SYN_SCHED_FLAG_MORE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &owner));

    /* (e) Full waiters[] array (SYN_MUTEX_MAX_WAITERS):
     * NORMAL waiters fill array; next LOW contender is ignored; CRITICAL replaces worst */
    SYN_Task pool[SYN_MUTEX_MAX_WAITERS];
    SYN_Task extra_low;
    SYN_Task extra_crit;
    syn_task_create(&extra_low, "ex_low", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&extra_crit, "ex_crit", dummy_pt_fn, SYN_PRIO_CRITICAL, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &owner));
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        syn_task_create(&pool[i], "pool", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &pool[i]));
    }
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, owner.priority);
    /* Lower-priority contender does not evict any NORMAL waiter */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &extra_low));
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        TEST_ASSERT_TRUE(m2.waiters[i] != &extra_low);
    }
    /* Higher-priority CRITICAL contender evicts a NORMAL waiter and boosts owner to CRITICAL */
    extra_crit.sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &extra_crit));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &owner));
    TEST_ASSERT_TRUE((extra_crit.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
}

static void test_mutex_transitive_inheritance_and_probe(void)
{
    /* 1. Transitive priority inheritance H(0) -> m2 -> M(2) -> m1 -> L(3),
     * even when L sits at index 0 ahead of M at index 1! */
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task tasks[3];
    SYN_Sched sched;

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    syn_task_create(&tasks[0], "low_l", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks[1], "med_m", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[2], "crit_h", dummy_pt_fn, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 3);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &tasks[1]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, tasks[0].priority);

    /* H contends on m2 held by M -> M and L both inherit CRITICAL! */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &tasks[2]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[0].priority);

    /* Demoting waiter H via syn_task_set_base_priority propagates down to M and L! */
    syn_task_set_base_priority(&tasks[2], SYN_PRIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[0].priority);

    /* Manual boost & restore on waiter H also propagates down to M and L! */
    syn_task_boost_priority(&tasks[2], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[0].priority);
    syn_task_restore_priority(&tasks[2]);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[0].priority);

    /* 2. Non-blocking PT_MUTEX_TRYLOCK / syn_mutex_probe does not falsely boost owner */
    SYN_Mutex m3;
    PT_MUTEX_INIT(&m3);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mutex_probe(NULL, &tasks[0]));
    TEST_ASSERT_TRUE(PT_MUTEX_TRYLOCK(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &tasks[1]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);

    syn_task_set_base_priority(&tasks[2], SYN_PRIO_CRITICAL);
    TEST_ASSERT_FALSE(PT_MUTEX_TRYLOCK(&m3, &tasks[2]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);
    TEST_ASSERT_NULL(m3.waiters[0]);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m3, &tasks[0]));
}

void run_mutex_tests(void)
{
    RUN_TEST(test_mutex_init_and_query);
    RUN_TEST(test_mutex_basic_trylock_and_unlock);
    RUN_TEST(test_mutex_standalone_lock_unlock);
    RUN_TEST(test_mutex_recursive_locking);
    RUN_TEST(test_mutex_priority_inheritance);
    RUN_TEST(test_mutex_priority_inversion_prevention);
    RUN_TEST(test_mutex_trylock_macro);
    RUN_TEST(test_mutex_boosted_deadline_and_suspended_waiter);
    RUN_TEST(test_mutex_multi_hold_unlink_and_saturation);
    RUN_TEST(test_mutex_release_on_task_exit_and_end);
    RUN_TEST(test_r4_mutex_unlock_share_refresh_and_multi_waiter_wake);
    RUN_TEST(test_r4_mutex_manual_boost_stale_waiter_and_full_array);
    RUN_TEST(test_mutex_transitive_inheritance_and_probe);
}
