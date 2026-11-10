/**
 * @file test_timer_wheel.c
 * @brief Unit test suite for Hashed Timing Wheel Scheduler (syn_timer_wheel).
 */

#include "syntropic/sched/syn_timer_wheel.h"
#include "unity/unity.h"

#include <string.h>

static int s_timer1_fired = 0;
static int s_timer2_fired = 0;

static void timer1_cb(void *arg)
{
    (void)arg;
    s_timer1_fired++;
}

static void timer2_cb(void *arg)
{
    (void)arg;
    s_timer2_fired++;
}

void test_timer_wheel_add_step_cancel(void)
{
    SYN_TimerWheel wheel;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));

    SYN_TimerWheelNode n1, n2;
    memset(&n1, 0, sizeof(n1));
    memset(&n2, 0, sizeof(n2));
    s_timer1_fired = 0;
    s_timer2_fired = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &n1, 3, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_timer_wheel_add(&wheel, &n2, 70, timer2_cb, NULL)); /* Multi-rotation timer */

    /* Step 1 & 2: no timers fire */
    TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(0, s_timer1_fired);

    /* Step 3: timer 1 fires */
    TEST_ASSERT_EQUAL_UINT32(1, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, s_timer1_fired);

    /* Cancel timer 2 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(&wheel, &n2));

    /* Advance wheel by 70 ticks */
    for (int i = 0; i < 70; i++) {
        syn_timer_wheel_step(&wheel);
    }
    TEST_ASSERT_EQUAL_INT(0, s_timer2_fired); /* Canceled, should not fire */

    /* Test collision: two timers in the same bucket */
    SYN_TimerWheelNode t_a, t_b, t_c;
    memset(&t_a, 0, sizeof(t_a));
    memset(&t_b, 0, sizeof(t_b));
    memset(&t_c, 0, sizeof(t_c));

    syn_timer_wheel_add(&wheel, &t_a, 5, timer1_cb, NULL);
    syn_timer_wheel_add(&wheel, &t_b, 5, timer2_cb, NULL);
    syn_timer_wheel_add(&wheel, &t_c, 5, timer1_cb, NULL);

    /* Re-add active timer t_b (triggers cancel before re-add) */
    syn_timer_wheel_add(&wheel, &t_b, 5, timer2_cb, NULL);

    /* Cancel middle element t_b */
    syn_timer_wheel_cancel(&wheel, &t_b);

    /* Step 5 ticks to fire t_a and t_c */
    for (int i = 0; i < 4; i++)
        syn_timer_wheel_step(&wheel);
    TEST_ASSERT_EQUAL_UINT32(2, syn_timer_wheel_step(&wheel));

    /* Null & param checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_add(NULL, &n1, 5, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_add(&wheel, &n1, 0, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(NULL, &n1));
    TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(NULL));

    /* Multi-rotation timer rotation_count decrement branch (line 90) */
    SYN_TimerWheelNode t_rot, t_rot2;
    memset(&t_rot, 0, sizeof(t_rot));
    memset(&t_rot2, 0, sizeof(t_rot2));
    syn_timer_wheel_add(&wheel, &t_rot2, 5, timer2_cb,
                        NULL); /* Added first -> ends up behind head */
    syn_timer_wheel_add(&wheel, &t_rot, SYN_TIMER_WHEEL_BUCKETS + 5, timer1_cb,
                        NULL); /* Prepended as head */
    for (int i = 0; i < 4; i++)
        syn_timer_wheel_step(&wheel);
    TEST_ASSERT_EQUAL_UINT32(1, syn_timer_wheel_step(&wheel)); /* t_rot2 fires, line 94 executed */
    TEST_ASSERT_TRUE(t_rot.active);                            /* Still active after 1st rotation */
    syn_timer_wheel_cancel(&wheel, &t_rot);
}

void test_timer_wheel_exact_bucket_multiple(void)
{
    SYN_TimerWheel wheel;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));

    SYN_TimerWheelNode n_64, n_128;
    memset(&n_64, 0, sizeof(n_64));
    memset(&n_128, 0, sizeof(n_128));
    s_timer1_fired = 0;
    s_timer2_fired = 0;

    /* Add timer 1 with delay = SYN_TIMER_WHEEL_BUCKETS (64 ticks) */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_timer_wheel_add(&wheel, &n_64, SYN_TIMER_WHEEL_BUCKETS, timer1_cb, NULL));

    /* Add timer 2 with delay = 2 * SYN_TIMER_WHEEL_BUCKETS (128 ticks) */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_timer_wheel_add(&wheel, &n_128, 2 * SYN_TIMER_WHEEL_BUCKETS, timer2_cb, NULL));

    /* Step ticks 1..63: neither should fire */
    for (uint32_t i = 1; i < SYN_TIMER_WHEEL_BUCKETS; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
        TEST_ASSERT_EQUAL_INT(0, s_timer1_fired);
        TEST_ASSERT_EQUAL_INT(0, s_timer2_fired);
    }

    /* Step tick 64: timer 1 MUST fire! */
    TEST_ASSERT_EQUAL_UINT32(1, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, s_timer1_fired);
    TEST_ASSERT_EQUAL_INT(0, s_timer2_fired);

    /* Step ticks 65..127: neither should fire */
    for (uint32_t i = SYN_TIMER_WHEEL_BUCKETS + 1; i < 2 * SYN_TIMER_WHEEL_BUCKETS; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
        TEST_ASSERT_EQUAL_INT(1, s_timer1_fired);
        TEST_ASSERT_EQUAL_INT(0, s_timer2_fired);
    }

    /* Step tick 128: timer 2 MUST fire! */
    TEST_ASSERT_EQUAL_UINT32(1, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, s_timer1_fired);
    TEST_ASSERT_EQUAL_INT(1, s_timer2_fired);
}

typedef struct {
    SYN_TimerWheel *wheel;
    SYN_TimerWheelNode a;
    SYN_TimerWheelNode b;
    SYN_TimerWheelNode c;
    int a_count;
    int b_count;
    int c_count;
    uint32_t readd_self_delay;
    uint32_t readd_peer_delay;
} WheelCbCtx;

static void cb_cancel_peer_b(void *arg)
{
    WheelCbCtx *ctx = (WheelCbCtx *)arg;
    ctx->a_count++;
    (void)syn_timer_wheel_cancel(ctx->wheel, &ctx->b);
    if (ctx->readd_self_delay > 0U) {
        (void)syn_timer_wheel_add(ctx->wheel, &ctx->a, ctx->readd_self_delay, cb_cancel_peer_b,
                                  ctx);
        ctx->readd_self_delay = 0U;
    }
    if (ctx->readd_peer_delay > 0U) {
        (void)syn_timer_wheel_add(ctx->wheel, &ctx->b, ctx->readd_peer_delay, timer2_cb, NULL);
        ctx->readd_peer_delay = 0U;
    }
}

static void cb_count_b(void *arg)
{
    WheelCbCtx *ctx = (WheelCbCtx *)arg;
    ctx->b_count++;
}

static void cb_count_c(void *arg)
{
    WheelCbCtx *ctx = (WheelCbCtx *)arg;
    ctx->c_count++;
}

static void test_r5_timer_wheel_callback_cancel_and_readd(void)
{
    SYN_TimerWheel wheel;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));

    /* Parameter validation edge cases */
    SYN_TimerWheelNode dummy;
    memset(&dummy, 0, sizeof(dummy));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_timer_wheel_add(&wheel, NULL, 1U, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_add(&wheel, &dummy, 1U, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(&wheel, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(&wheel, &dummy));

    /* Multiple multi-rotation nodes in the same bucket preserve rotation & order */
    SYN_TimerWheelNode rot1;
    SYN_TimerWheelNode rot2;
    memset(&rot1, 0, sizeof(rot1));
    memset(&rot2, 0, sizeof(rot2));
    s_timer1_fired = 0;
    s_timer2_fired = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_timer_wheel_add(&wheel, &rot1, SYN_TIMER_WHEEL_BUCKETS + 1U, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK,
        syn_timer_wheel_add(&wheel, &rot2, (2U * SYN_TIMER_WHEEL_BUCKETS) + 1U, timer2_cb, NULL));
    TEST_ASSERT_EQUAL_size_t(0U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(&wheel, &rot1));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(&wheel, &rot2));

    /* Case 1: Same-bucket C -> B -> A (added C, B, A so head order is A -> B -> C).
     * A cancels B AND re-adds A to the SAME bucket (delay = 64) AND re-adds B to delay = 2!
     * Expected on tick 1: A fires, cancelled B does NOT fire, C is NOT orphaned and fires! */
    WheelCbCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.wheel = &wheel;
    ctx.readd_self_delay = SYN_TIMER_WHEEL_BUCKETS;
    ctx.readd_peer_delay = 2U;
    s_timer2_fired = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.c, 1U, cb_count_c, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.b, 1U, cb_count_b, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.a, 1U, cb_cancel_peer_b, &ctx));

    TEST_ASSERT_EQUAL_size_t(2U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, ctx.a_count);
    TEST_ASSERT_EQUAL_INT(0, ctx.b_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.c_count);

    /* Re-added B fires 2 ticks later without cross-linking */
    TEST_ASSERT_EQUAL_size_t(0U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_size_t(1U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, s_timer2_fired);

    /* Re-added A fires on tick 64 after its re-add (62 more steps) */
    for (uint32_t i = 0U; i < SYN_TIMER_WHEEL_BUCKETS - 3U; i++) {
        TEST_ASSERT_EQUAL_size_t(0U, syn_timer_wheel_step(&wheel));
    }
    TEST_ASSERT_EQUAL_size_t(1U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(2, ctx.a_count);
}

typedef struct {
    SYN_TimerWheel *wheel;
    SYN_TimerWheelNode a;
    SYN_TimerWheelNode b;
    SYN_TimerWheelNode c;
    SYN_TimerWheelNode x;
    SYN_TimerWheelNode y;
    SYN_TimerWheelNode z_head;
    size_t inner_fired;
    int a_count;
    int b_count;
    int c_count;
    int x_count;
    int y_count;
    int z_count;
} ReentrantWheelCtx;

static void reent_step_cb(void *arg)
{
    ReentrantWheelCtx *ctx = (ReentrantWheelCtx *)arg;
    ctx->a_count++;
    ctx->inner_fired = syn_timer_wheel_step(ctx->wheel);
}

static void reent_inner_x_cb(void *arg)
{
    ReentrantWheelCtx *ctx = (ReentrantWheelCtx *)arg;
    ctx->x_count++;
    /* Cancel outer pending head B AND future bucket head Z while inside nested step */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(ctx->wheel, &ctx->b));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(ctx->wheel, &ctx->z_head));
}

static void reent_b_cb(void *arg)
{
    ((ReentrantWheelCtx *)arg)->b_count++;
}

static void reent_c_cb(void *arg)
{
    ((ReentrantWheelCtx *)arg)->c_count++;
}

static void reent_y_cb(void *arg)
{
    ((ReentrantWheelCtx *)arg)->y_count++;
}

static void reent_z_cb(void *arg)
{
    ((ReentrantWheelCtx *)arg)->z_count++;
}

static void test_timer_wheel_reentrant_step_and_bucket_head_cancel(void)
{
    SYN_TimerWheel wheel;
    ReentrantWheelCtx ctx;

    /* 1. Re-entrant step() onto an empty bucket preserves remaining outer expired list (B, C) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));
    memset(&ctx, 0, sizeof(ctx));
    ctx.wheel = &wheel;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.c, 1U, reent_c_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.b, 1U, reent_b_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.a, 1U, reent_step_cb, &ctx));

    TEST_ASSERT_EQUAL_size_t(3U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_size_t(0U, ctx.inner_fired);
    TEST_ASSERT_EQUAL_INT(1, ctx.a_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.b_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.c_count);
    TEST_ASSERT_NULL(wheel.expired_head);

    /* 2. Re-entrant step() onto non-empty bucket [X, Y] where X cancels outer pending head B
     * and future bucket head Z: X and Y fire inner, cancelled B skips, C fires outer */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));
    memset(&ctx, 0, sizeof(ctx));
    ctx.wheel = &wheel;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.c, 1U, reent_c_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.b, 1U, reent_b_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.a, 1U, reent_step_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.y, 2U, reent_y_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.x, 2U, reent_inner_x_cb, &ctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &ctx.z_head, 3U, reent_z_cb, &ctx));

    TEST_ASSERT_EQUAL_size_t(2U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_size_t(2U, ctx.inner_fired);
    TEST_ASSERT_EQUAL_INT(1, ctx.a_count);
    TEST_ASSERT_EQUAL_INT(0, ctx.b_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.c_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.x_count);
    TEST_ASSERT_EQUAL_INT(1, ctx.y_count);
    TEST_ASSERT_NULL(wheel.expired_head);

    /* Step tick 3: cancelled bucket-head z_head must not fire */
    TEST_ASSERT_EQUAL_size_t(0U, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(0, ctx.z_count);
}

typedef struct {
    SYN_TimerWheel *wheel;
    SYN_TimerWheelNode post_init_node;
    int reinit_cb_runs;
    int second_cb_runs;
    int post_init_runs;
} WheelReinitCtx;

static void post_init_cb(void *arg)
{
    ((WheelReinitCtx *)arg)->post_init_runs++;
}

static void reinit_wheel_in_cb(void *arg)
{
    WheelReinitCtx *ctx = (WheelReinitCtx *)arg;
    ctx->reinit_cb_runs++;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(ctx->wheel));
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_timer_wheel_add(ctx->wheel, &ctx->post_init_node, 1U, post_init_cb, ctx));
}

static void never_reached_after_reinit_cb(void *arg)
{
    ((WheelReinitCtx *)arg)->second_cb_runs++;
}

static void test_timer_wheel_cross_wheel_reject_and_reinit_in_cb(void)
{
    SYN_TimerWheel wheel_a;
    SYN_TimerWheel wheel_b;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel_a));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel_b));

    SYN_TimerWheelNode n_tail, n_head, b_other;
    memset(&n_tail, 0, sizeof(n_tail));
    memset(&n_head, 0, sizeof(n_head));
    memset(&b_other, 0, sizeof(b_other));
    s_timer1_fired = 0;
    s_timer2_fired = 0;

    /* Populate wheel_a bucket 1 with [n_head -> n_tail] and wheel_b bucket 1 with [b_other] */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel_a, &n_tail, 1U, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel_a, &n_head, 1U, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel_b, &b_other, 1U, timer2_cb, NULL));

    /* Cancelling or adding wheel_a's head OR non-head node against wheel_b must fail cleanly */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(&wheel_b, &n_head));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(&wheel_b, &n_tail));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_timer_wheel_add(&wheel_b, &n_head, 2U, timer2_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_timer_wheel_add(&wheel_b, &n_tail, 2U, timer2_cb, NULL));

    /* Both wheels remain intact and fire their own timers */
    TEST_ASSERT_EQUAL_size_t(2U, syn_timer_wheel_step(&wheel_a));
    TEST_ASSERT_EQUAL_INT(2, s_timer1_fired);
    TEST_ASSERT_EQUAL_size_t(1U, syn_timer_wheel_step(&wheel_b));
    TEST_ASSERT_EQUAL_INT(1, s_timer2_fired);

    /* Callback calling syn_timer_wheel_init(wheel) mid-step aborts stale list without NULL deref */
    WheelReinitCtx rctx;
    memset(&rctx, 0, sizeof(rctx));
    rctx.wheel = &wheel_a;
    SYN_TimerWheelNode first_exp, second_exp;
    memset(&first_exp, 0, sizeof(first_exp));
    memset(&second_exp, 0, sizeof(second_exp));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel_a));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel_a, &second_exp, 1U,
                                                      never_reached_after_reinit_cb, &rctx));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_timer_wheel_add(&wheel_a, &first_exp, 1U, reinit_wheel_in_cb, &rctx));

    TEST_ASSERT_EQUAL_size_t(1U, syn_timer_wheel_step(&wheel_a));
    TEST_ASSERT_EQUAL_INT(1, rctx.reinit_cb_runs);
    TEST_ASSERT_EQUAL_INT(0, rctx.second_cb_runs);
    TEST_ASSERT_NULL(wheel_a.expired_head);
    TEST_ASSERT_FALSE(second_exp.active);
    TEST_ASSERT_NULL(second_exp.next);
    TEST_ASSERT_NULL(second_exp.prev);

    /* Post-reinit node added inside the callback fires cleanly on the next tick */
    TEST_ASSERT_EQUAL_size_t(1U, syn_timer_wheel_step(&wheel_a));
    TEST_ASSERT_EQUAL_INT(1, rctx.post_init_runs);
}

void run_timer_wheel_tests(void)
{
    RUN_TEST(test_timer_wheel_add_step_cancel);
    RUN_TEST(test_timer_wheel_exact_bucket_multiple);
    RUN_TEST(test_r5_timer_wheel_callback_cancel_and_readd);
    RUN_TEST(test_timer_wheel_reentrant_step_and_bucket_head_cancel);
    RUN_TEST(test_timer_wheel_cross_wheel_reject_and_reinit_in_cb);
}
