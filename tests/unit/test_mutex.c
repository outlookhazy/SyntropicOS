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

static void test_tier_a_mutex_evicted_waiter_wakeup_isolation(void)
{
    /* SCHED-3: Unlocking m2 wakes > SYN_MUTEX_MAX_WAITERS evicted waiters to WORK rank
     * without stripping SYN_SCHED_FLAG_MUTEX_WAIT or inheritance boost from m1! */
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task tasks[8];
    SYN_Sched sched;

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    mock_tick_ms = 0U;

    syn_task_create(&tasks[0], "own1", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks[1], "w_m1", dummy_pt_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[2], "own2", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        syn_task_create(&tasks[3U + i], "c_norm", dummy_pt_fn, SYN_PRIO_NORMAL, NULL);
    }
    syn_task_create(&tasks[7], "c_evict", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 8U);

    /* Lock m1 with own1, contend m1 with w_m1 -> own1 boosted to HIGH */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[0]));
    tasks[1].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[0].priority);

    /* Lock m2 with own2; contend first with LOW c_evict, then 4 NORMAL tasks to evict c_evict */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &tasks[2]));
    tasks[7].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &tasks[7]));
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        tasks[3U + i].sched_flags = SYN_SCHED_FLAG_PASSED;
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &tasks[3U + i]));
    }

    /* Unlock m2: all 4 tracked waiters AND evicted tasks[7] promote to MORE */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &tasks[2]));
    for (size_t i = 0U; i < SYN_MUTEX_MAX_WAITERS; i++) {
        TEST_ASSERT_TRUE((tasks[3U + i].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    }
    TEST_ASSERT_TRUE((tasks[7].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[7].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[7].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);

    /* Unrelated m1 waiter w_m1 keeps MUTEX_WAIT and own1 keeps HIGH boost! */
    TEST_ASSERT_TRUE((tasks[1].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[0].priority);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);
}

static void test_mutex_pi_cycle_demotion_and_unlock_recovery(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Mutex m3;
    SYN_Task tasks[3];
    SYN_Sched sched;

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    PT_MUTEX_INIT(&m3);
    mock_tick_ms = 0U;

    syn_task_create(&tasks[0], "low_a", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks[1], "low_b", dummy_pt_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks[2], "crit_c", dummy_pt_fn, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 3U);

    /* A holds m1 + m3, B holds m2; A <-> B non-blocking try_lock cycle */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &tasks[1]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);

    /* External CRITICAL waiter C contends on m3 -> boosts both A and B to CRITICAL */
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m3, &tasks[2]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);

    /* Suspending C drops the cycle back to intrinsic LOW (no self-reinforcing pin!) */
    syn_task_suspend(&tasks[2]);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);

    /* Re-contend C on m3, then unlock m3 -> A and B again relax back to LOW */
    syn_task_resume(&tasks[2]);
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m3, &tasks[2]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &tasks[1]));
}

typedef struct {
    SYN_Mutex *mtx;
    int step;
} ContendContinueCtx;

static SYN_PT_Status crit_contend_continue_fn(SYN_PT *pt, SYN_Task *task)
{
    ContendContinueCtx *ctx = (ContendContinueCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->step = 1;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(ctx->mtx, task));
    PT_CONTINUE(pt); /* Yields PT_MORE while still contending! */
    ctx->step = 2;
    PT_PASS(pt); /* Next dispatch stops contending */
    PT_END(pt);
}

static SYN_PT_Status owner_pass_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        PT_PASS(pt);
    }
    PT_END(pt);
}

static void test_mutex_try_lock_pt_continue_and_standalone_unlock_wake(void)
{
    /* 1. Contending task yielding PT_CONTINUE (PT_MORE) retains MUTEX_WAIT + owner boost */
    SYN_Mutex m1;
    PT_MUTEX_INIT(&m1);
    ContendContinueCtx ctx = {&m1, 0};
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "crit_more", crit_contend_continue_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "low_own", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 2U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[1]));

    /* Dispatch 1: crit_more contends on m1 and returns PT_MORE -> owner boosted to CRITICAL */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, ctx.step);
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);

    /* Dispatch 2: RR at CRITICAL runs boosted owner low_own; crit_more still holds MUTEX_WAIT */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, ctx.step);
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);

    /* Dispatch 3: crit_more stops contending and returns PT_IDLE -> owner drops back to LOW */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, ctx.step);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[1]));

    /* 2. Standalone (owner == NULL) mutex records task waiters, wakes them to MORE on unlock,
     * and is isolated from normal (non-overflowing) unlocks of task-owned mutexes */
    SYN_Mutex m_standalone;
    SYN_Mutex m_other;
    PT_MUTEX_INIT(&m_standalone);
    PT_MUTEX_INIT(&m_other);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_standalone, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_other, &tasks[1]));

    tasks[0].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_standalone, &tasks[0]));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);

    /* Unlocking unrelated task-owned m_other does NOT clobber tasks[0]'s standalone wait */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_other, &tasks[1]));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED) != 0U);
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);

    /* Unlocking m_standalone promotes tasks[0] from PASSED to MORE and clears MUTEX_WAIT */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_standalone, NULL));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
}

typedef struct {
    SYN_Mutex *m1;
    SYN_Mutex *m2;
    int phase;
} MutexSwitchCtx;

static SYN_PT_Status crit_switch_mutex_fn(SYN_PT *pt, SYN_Task *task)
{
    MutexSwitchCtx *ctx = (MutexSwitchCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->phase = 1;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(ctx->m1, task));
    PT_CONTINUE(pt);
    ctx->phase = 2;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(ctx->m2, task));
    PT_CONTINUE(pt);
    ctx->phase = 3;
    PT_PASS(pt);
    PT_END(pt);
}

static void test_mutex_switch_and_dual_contention_pi(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Mutex m3;
    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    PT_MUTEX_INIT(&m3);

    MutexSwitchCtx ctx = {&m1, &m2, 0};
    SYN_Task tasks[3];
    SYN_Sched sched;

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "crit_sw", crit_switch_mutex_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "own1", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks[2], "own2", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 3U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[1]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m2, &tasks[2]));

    /* Dispatch 1: crit_sw contends on m1 and returns PT_MORE -> own1 boosted to CRITICAL */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, ctx.phase);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[2].priority);

    /* Dispatch 2: RR at CRITICAL runs boosted own1 */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, ctx.phase);

    /* Dispatch 3: crit_sw switches contention from m1 to m2 -> own1 drops to LOW immediately */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, ctx.phase);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);
    TEST_ASSERT_NULL(m1.waiters[0]);
    TEST_ASSERT_EQUAL_PTR(&tasks[0], m2.waiters[0]);

    /* Unlocking abandoned m1 does NOT drop crit_sw's MUTEX_WAIT or own2's CRITICAL boost */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[1]));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);

    /* Dual contention in a single step + duplicate try_lock while holding m3 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m1, &tasks[1]));
    tasks[0].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m2, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);

    /* Unlocking m1 wakes crit_sw to MORE while keeping MUTEX_WAIT and own2 boosted on m2 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m1, &tasks[1]));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);

    /* Unlocking m2 clears crit_sw's final MUTEX_WAIT and restores own2 to LOW */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m2, &tasks[2]));
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[2].priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m3, &tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m3, &tasks[0]));
}

static void test_mutex_standalone_evicted_wake_and_unbound_owner_refresh(void)
{
    SYN_Task tasks[6];
    SYN_Sched sched;
    SYN_Mutex m_sa1;
    SYN_Mutex m_sa2;
    SYN_Mutex m_ext;
    SYN_Mutex m_unb;
    PT_MUTEX_INIT(&m_sa1);
    PT_MUTEX_INIT(&m_sa2);
    PT_MUTEX_INIT(&m_ext);
    PT_MUTEX_INIT(&m_unb);

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "w_evict", owner_pass_fn, SYN_PRIO_LOW, NULL);
    for (int i = 1; i < 6; i++) {
        syn_task_create(&tasks[i], "w_high", owner_pass_fn, SYN_PRIO_HIGH, NULL);
    }
    syn_sched_init(&sched, tasks, 6U);

    /* Standalone overflow + suspended direct waiters + peer waiting on second standalone mutex */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_sa1, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_sa2, NULL));

    tasks[5].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_sa2, &tasks[5]));

    tasks[0].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_sa1, &tasks[0]));
    for (int i = 1; i <= 4; i++) {
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_sa1, &tasks[i]));
    }
    TEST_ASSERT_TRUE(m_sa1.has_evicted);

    /* Suspend all 4 direct waiters; verify try_lock by a suspended task does NOT set MUTEX_WAIT */
    for (int i = 1; i <= 4; i++) {
        syn_task_suspend(&tasks[i]);
    }
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_sa1, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[1].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);

    /* Unlocking m_sa1 wakes evicted tasks[0] via wait_sched while keeping tasks[5] on m_sa2 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_sa1, NULL));
    TEST_ASSERT_TRUE((tasks[0].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_TRUE((tasks[5].sched_flags & SYN_SCHED_FLAG_PASSED) != 0U);
    TEST_ASSERT_TRUE((tasks[5].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_sa2, NULL));
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[5].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_NULL(sched.standalone_mutexes);

    /* Unbound owner (ext_owner.sched == NULL) unlock refreshes bound waiter's scheduler share */
    SYN_Task ext_owner = {0};
    syn_task_create(&ext_owner, "ext", owner_pass_fn, SYN_PRIO_LOW, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_ext, &ext_owner));
    tasks[5].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_ext, &tasks[5]));
    sched.running = &tasks[0];
    sched.share_contended = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_ext, &ext_owner));
    TEST_ASSERT_TRUE(sched.share_contended);
    sched.running = NULL;

    /* Unbound overflow (wake_sched == NULL) safely skips evicted scheduler scan */
    SYN_Task unb[5];
    for (int i = 0; i < 5; i++) {
        syn_task_create(&unb[i], "unb", owner_pass_fn, (i == 0) ? SYN_PRIO_LOW : SYN_PRIO_HIGH,
                        NULL);
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_unb, &ext_owner));
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_unb, &unb[i]));
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_unb, &ext_owner));
}

static void test_mutex_recursive_relock_wait_keep_and_cross_sched_pi(void)
{
    mock_tick_ms = 0U;

    /* 1. Recursive re-lock on an already-owned mutex preserves active MUTEX_WAIT + owner boost */
    SYN_Mutex m_rec;
    SYN_Mutex m_contended;
    SYN_Task rec_tasks[2];
    SYN_Sched rec_sched;
    PT_MUTEX_INIT(&m_rec);
    PT_MUTEX_INIT(&m_contended);

    syn_task_create(&rec_tasks[0], "crit_rec", owner_pass_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&rec_tasks[1], "low_own", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&rec_sched, rec_tasks, 2U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_rec, &rec_tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_contended, &rec_tasks[1]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_contended, &rec_tasks[0]));
    TEST_ASSERT_TRUE((rec_tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, rec_tasks[1].priority);

    /* Case 2 recursive re-lock on m_rec must NOT clear rec_tasks[0]'s wait on m_contended */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_rec, &rec_tasks[0]));
    TEST_ASSERT_EQUAL_UINT16(2U, m_rec.lock_count);
    TEST_ASSERT_TRUE((rec_tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, rec_tasks[1].priority);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_contended, &rec_tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(0U, rec_tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, rec_tasks[1].priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_rec, &rec_tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_rec, &rec_tasks[0]));

    /* 2. Unbound owner (ext_owner.sched == NULL) holding TWO contended mutexes + standalone NULL:
     *    - Contend m_ext1 FIRST, m_ext2 SECOND, m_null THIRD -> standalone_mutexes =
     * m_null->m_ext2->m_ext1
     *    - Unlock m_ext1 FIRST -> walks next_contended tail and keeps waiter on m_ext2
     *    - Boost/restore waiter on m_ext2 -> propagates boost & demotion to unbound ext_owner! */
    SYN_Task ext_owner;
    syn_task_create(&ext_owner, "ext_unb", owner_pass_fn, SYN_PRIO_LOW, NULL);
    SYN_Mutex m_ext1;
    SYN_Mutex m_ext2;
    SYN_Mutex m_null;
    PT_MUTEX_INIT(&m_ext1);
    PT_MUTEX_INIT(&m_ext2);
    PT_MUTEX_INIT(&m_null);

    SYN_Task w_unb;
    SYN_Sched unb_sched;
    syn_task_create(&w_unb, "w_unb", owner_pass_fn, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&unb_sched, &w_unb, 1U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_ext1, &ext_owner));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_ext2, &ext_owner));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_null, NULL));

    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_ext1, &w_unb));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_ext2, &w_unb));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_null, &w_unb));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, ext_owner.priority);

    /* Unlock tail m_ext1: exercises mutex_unlink_contended list traversal while keeping m_ext2 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_ext1, &ext_owner));
    TEST_ASSERT_TRUE((w_unb.sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, ext_owner.priority);

    /* Boost & restore w_unb -> exercises unbound owner boost & demotion + owner==NULL skip */
    syn_task_boost_priority(&w_unb, SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, ext_owner.priority);
    syn_task_restore_priority(&w_unb);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, ext_owner.priority);

    /* Suspend w_unb -> unbound ext_owner drops back to intrinsic SYN_PRIO_LOW! */
    syn_task_suspend(&w_unb);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, ext_owner.priority);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_ext2, &ext_owner));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_null, NULL));

    /* 3. Cross-scheduler PI: sched2 waiter -> m_shared -> sched1 owner -> m_s1_chain -> s1_root */
    SYN_Task s1_tasks[3];
    SYN_Task s2_tasks[6];
    SYN_Sched sched1;
    SYN_Sched sched2;
    SYN_Mutex m_shared;
    SYN_Mutex m_s1_chain;
    SYN_Mutex m_s2_local;
    PT_MUTEX_INIT(&m_shared);
    PT_MUTEX_INIT(&m_s1_chain);
    PT_MUTEX_INIT(&m_s2_local);

    syn_task_create(&s1_tasks[0], "s1_root", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&s1_tasks[1], "s1_mid", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&s1_tasks[2], "s1_run", owner_pass_fn, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&sched1, s1_tasks, 3U);

    syn_task_create(&s2_tasks[0], "s2_evict", owner_pass_fn, SYN_PRIO_LOW, NULL);
    for (int i = 1; i < 5; i++) {
        syn_task_create(&s2_tasks[i], "s2_high", owner_pass_fn, SYN_PRIO_HIGH, NULL);
    }
    syn_task_create(&s2_tasks[5], "s2_lown", owner_pass_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched2, s2_tasks, 6U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_s1_chain, &s1_tasks[0]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_shared, &s1_tasks[1]));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_s1_chain, &s1_tasks[1]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(&m_s2_local, &s2_tasks[5]));

    /* Evict s2_tasks[0] with 4 HIGH waiters from sched2; s2_tasks[1] also waits on m_s2_local */
    s2_tasks[0].sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_shared, &s2_tasks[0]));
    for (int i = 1; i <= 4; i++) {
        s2_tasks[i].sched_flags = SYN_SCHED_FLAG_PASSED;
        TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_shared, &s2_tasks[i]));
    }
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_mutex_try_lock(&m_s2_local, &s2_tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, s1_tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, s1_tasks[0].priority);

    /* Boosting s2_tasks[1] to CRITICAL boosts s1_mid & s1_root in sched1 and refreshes sched1! */
    sched1.running = &s1_tasks[2];
    sched1.share_contended = false;
    syn_task_boost_priority(&s2_tasks[1], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, s1_tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, s1_tasks[0].priority);
    TEST_ASSERT_TRUE(sched1.share_contended);

    /* Restoring s2_tasks[1] demotes s1_mid & s1_root back to HIGH across schedulers! */
    syn_task_restore_priority(&s2_tasks[1]);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, s1_tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, s1_tasks[0].priority);
    sched1.running = NULL;

    /* Unlocking m_shared wakes sched2 evicted s2_tasks[0] while keeping s2_tasks[1] on m_s2_local
     */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_shared, &s1_tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, s1_tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, s1_tasks[0].priority);
    TEST_ASSERT_TRUE((s2_tasks[0].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_EQUAL_UINT8(0U, s2_tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_TRUE((s2_tasks[1].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT) != 0U);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_s2_local, &s2_tasks[5]));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_unlock(&m_s1_chain, &s1_tasks[0]));
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
    RUN_TEST(test_tier_a_mutex_evicted_waiter_wakeup_isolation);
    RUN_TEST(test_mutex_pi_cycle_demotion_and_unlock_recovery);
    RUN_TEST(test_mutex_try_lock_pt_continue_and_standalone_unlock_wake);
    RUN_TEST(test_mutex_switch_and_dual_contention_pi);
    RUN_TEST(test_mutex_standalone_evicted_wake_and_unbound_owner_refresh);
    RUN_TEST(test_mutex_recursive_relock_wait_keep_and_cross_sched_pi);
}
