/**
 * @file test_sequencer.c
 * @brief Unity tests for syn_sequencer.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_sequencer.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int seq_action_count = 0;
static void seq_action_a(void *ctx)
{
    (void)ctx;
    seq_action_count++;
}

static int seq_complete_count = 0;
static void seq_on_done(SYN_Sequencer *seq, void *ctx)
{
    (void)seq;
    (void)ctx;
    seq_complete_count++;
}

static void test_sequencer(void)
{
    mock_tick_ms = 0;
    seq_action_count = 0;
    seq_complete_count = 0;

    static const SYN_SeqStep steps[] = {
        {seq_action_a, NULL, 0},  /* step 0: action, no delay */
        {NULL, NULL, 100},        /* step 1: delay 100ms      */
        {seq_action_a, NULL, 50}, /* step 2: action + 50ms    */
        {seq_action_a, NULL, 0},  /* step 3: action, finish   */
    };

    SYN_Sequencer seq;
    syn_seq_init(&seq, steps, 4);
    syn_seq_on_complete(&seq, seq_on_done, NULL);

    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));

    syn_seq_start(&seq);
    TEST_ASSERT_FALSE(syn_seq_is_done(&seq));

    /* First update: runs step 0 (action), step 1 starts waiting */
    syn_seq_update(&seq);
    TEST_ASSERT_EQUAL_INT(1, seq_action_count);

    /* Not enough time for step 1's delay */
    mock_tick_advance(50);
    syn_seq_update(&seq);
    TEST_ASSERT_EQUAL_INT(1, seq_action_count);

    /* Step 1 delay complete, step 2 runs (action + starts delay) */
    mock_tick_advance(60);
    syn_seq_update(&seq);
    TEST_ASSERT_EQUAL_INT(2, seq_action_count);

    /* Step 2 delay done, step 3 runs, sequence done */
    mock_tick_advance(55);
    bool done = syn_seq_update(&seq);
    TEST_ASSERT_EQUAL_INT(3, seq_action_count);
    TEST_ASSERT_TRUE(done);
    TEST_ASSERT_EQUAL_INT(1, seq_complete_count);
    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));
    TEST_ASSERT_EQUAL_INT(1, syn_seq_loops(&seq));

    /* Loop mode */
    seq_action_count = 0;
    syn_seq_set_loop(&seq, true);
    syn_seq_start(&seq);
    syn_seq_update(&seq);
    mock_tick_advance(200);
    syn_seq_update(&seq);
    mock_tick_advance(200);
    syn_seq_update(&seq);
    /* Should have looped at least once */
    TEST_ASSERT_TRUE(syn_seq_loops(&seq) >= 2);
    syn_seq_stop(&seq);
    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));
}

/** syn_seq_update on DONE/IDLE state — exercises line 85: returns false */
static void test_sequencer_update_when_done(void)
{
    mock_tick_ms = 0;
    static const SYN_SeqStep steps[] = {
        {seq_action_a, NULL, 0},
    };
    SYN_Sequencer seq;
    syn_seq_init(&seq, steps, 1);

    /* Before start: IDLE → update returns false */
    bool result = syn_seq_update(&seq);
    TEST_ASSERT_FALSE(result);

    /* Run to completion */
    syn_seq_start(&seq);
    syn_seq_update(&seq);
    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));

    /* After done: update again returns false */
    result = syn_seq_update(&seq);
    TEST_ASSERT_FALSE(result);
}

static void test_sequencer_getters_and_uninitialized_update(void)
{
    TEST_ASSERT_FALSE(syn_seq_update(NULL));

    SYN_Sequencer seq;
    memset(&seq, 0, sizeof(seq));
    TEST_ASSERT_EQUAL_UINT16(0, syn_seq_current_step(&seq));
    TEST_ASSERT_FALSE(syn_seq_update(&seq)); /* steps is NULL */
}

static bool g_restarted_once = false;

static void seq_stop_self_action(void *ctx)
{
    SYN_Sequencer *seq = (SYN_Sequencer *)ctx;
    seq_action_count++;
    syn_seq_stop(seq);
}

static void seq_restart_once_action(void *ctx)
{
    SYN_Sequencer *seq = (SYN_Sequencer *)ctx;
    seq_action_count++;
    if (!g_restarted_once) {
        g_restarted_once = true;
        syn_seq_start(seq);
    }
}

static void seq_stop_on_done_cb(SYN_Sequencer *seq, void *ctx)
{
    (void)ctx;
    seq_complete_count++;
    syn_seq_stop(seq);
}

static void seq_restart_once_on_done_cb(SYN_Sequencer *seq, void *ctx)
{
    int *restarts = (int *)ctx;
    TEST_ASSERT_TRUE(syn_seq_is_done(seq));
    if (*restarts == 0) {
        (*restarts)++;
        syn_seq_start(seq);
    }
}

static void test_sequencer_mid_step_stop_and_restart(void)
{
    mock_tick_ms = 0U;
    seq_action_count = 0;
    seq_complete_count = 0;

    /* 1. Action callback calls syn_seq_stop(seq) mid-step (with non-zero delay_ms):
     * state stays SYN_SEQ_IDLE, current stays 0, does not overwrite to SYN_SEQ_WAITING */
    SYN_Sequencer stop_seq;
    SYN_SeqStep stop_steps[2] = {
        {seq_stop_self_action, &stop_seq, 100U},
        {seq_action_a, NULL, 0U},
    };
    syn_seq_init(&stop_seq, stop_steps, 2U);
    syn_seq_start(&stop_seq);

    TEST_ASSERT_FALSE(syn_seq_update(&stop_seq));
    TEST_ASSERT_EQUAL_INT(1, seq_action_count);
    TEST_ASSERT_EQUAL_INT(SYN_SEQ_IDLE, stop_seq.state);
    TEST_ASSERT_TRUE(syn_seq_is_done(&stop_seq));
    TEST_ASSERT_EQUAL_UINT16(0U, syn_seq_current_step(&stop_seq));

    /* Advancing past 100 ms does NOT resume because state is IDLE, not WAITING */
    mock_tick_advance(200U);
    TEST_ASSERT_FALSE(syn_seq_update(&stop_seq));
    TEST_ASSERT_EQUAL_INT(1, seq_action_count);

    /* 2. Action callback at step 1 calls syn_seq_start(seq) (seq->current != step_idx) */
    seq_action_count = 0;
    g_restarted_once = false;
    SYN_Sequencer restart_seq;
    SYN_SeqStep restart_steps[2] = {
        {seq_action_a, NULL, 0U},
        {seq_restart_once_action, &restart_seq, 50U},
    };
    syn_seq_init(&restart_seq, restart_steps, 2U);
    syn_seq_start(&restart_seq);

    /* Pass 1: step 0 runs, step 1 restarts to step 0, step 0 runs again (max_exec = 3 reached) */
    TEST_ASSERT_FALSE(syn_seq_update(&restart_seq));
    TEST_ASSERT_EQUAL_INT(3, seq_action_count);
    TEST_ASSERT_EQUAL_UINT16(1U, syn_seq_current_step(&restart_seq));
    TEST_ASSERT_EQUAL_INT(SYN_SEQ_RUNNING, restart_seq.state);

    /* Pass 2: step 1 runs without restarting and enters 50 ms wait */
    TEST_ASSERT_FALSE(syn_seq_update(&restart_seq));
    TEST_ASSERT_EQUAL_INT(4, seq_action_count);
    TEST_ASSERT_EQUAL_INT(SYN_SEQ_WAITING, restart_seq.state);

    mock_tick_advance(50U);
    TEST_ASSERT_TRUE(syn_seq_update(&restart_seq));
    TEST_ASSERT_TRUE(syn_seq_is_done(&restart_seq));

    /* 3. on_complete callback stops a looping sequencer mid-completion */
    seq_complete_count = 0;
    syn_seq_set_loop(&restart_seq, true);
    syn_seq_on_complete(&restart_seq, seq_stop_on_done_cb, NULL);
    syn_seq_start(&restart_seq);
    syn_seq_update(&restart_seq);
    mock_tick_advance(50U);
    TEST_ASSERT_FALSE(syn_seq_update(&restart_seq));
    TEST_ASSERT_EQUAL_INT(1, seq_complete_count);
    TEST_ASSERT_EQUAL_INT(SYN_SEQ_IDLE, restart_seq.state);

    /* 4. Non-looping sequencer restarted inside on_complete stays RUNNING */
    int done_restarts = 0;
    seq_action_count = 0;
    syn_seq_set_loop(&restart_seq, false);
    syn_seq_on_complete(&restart_seq, seq_restart_once_on_done_cb, &done_restarts);
    syn_seq_start(&restart_seq);
    TEST_ASSERT_FALSE(syn_seq_update(&restart_seq));
    mock_tick_advance(50U);
    TEST_ASSERT_FALSE(syn_seq_update(&restart_seq));
    TEST_ASSERT_EQUAL_INT(1, done_restarts);
    TEST_ASSERT_FALSE(syn_seq_is_done(&restart_seq));
    mock_tick_advance(50U);
    TEST_ASSERT_TRUE(syn_seq_update(&restart_seq));
    TEST_ASSERT_TRUE(syn_seq_is_done(&restart_seq));
}

void run_sequencer_tests(void)
{
    RUN_TEST(test_sequencer);
    RUN_TEST(test_sequencer_update_when_done);
    RUN_TEST(test_sequencer_getters_and_uninitialized_update);
    RUN_TEST(test_sequencer_mid_step_stop_and_restart);
}
