/**
 * @file test_sequencer.c
 * @brief Unity tests for syn_sequencer.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_sequencer.h"

static int seq_action_count = 0;
static void seq_action_a(void *ctx) { (void)ctx; seq_action_count++; }

static int seq_complete_count = 0;
static void seq_on_done(SYN_Sequencer *seq, void *ctx)
{
    (void)seq; (void)ctx;
    seq_complete_count++;
}

static void test_sequencer(void)
{

    mock_tick_ms = 0;
    seq_action_count = 0;
    seq_complete_count = 0;

    static const SYN_SeqStep steps[] = {
        { seq_action_a, NULL, 0 },     /* step 0: action, no delay */
        { NULL,         NULL, 100 },   /* step 1: delay 100ms      */
        { seq_action_a, NULL, 50 },    /* step 2: action + 50ms    */
        { seq_action_a, NULL, 0 },     /* step 3: action, finish   */
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

void run_sequencer_tests(void)
{
    RUN_TEST(test_sequencer);
}
