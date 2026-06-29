/**
 * @file test_fsm.c
 * @brief Unity tests for syn_fsm.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_fsm.h"

static char log_capture_buf[1024];
static size_t log_capture_pos = 0;

static void log_capture_output(const char *str, size_t len)
{
    size_t space = sizeof(log_capture_buf) - log_capture_pos - 1;
    if (len > space) len = space;
    memcpy(log_capture_buf + log_capture_pos, str, len);
    log_capture_pos += len;
    log_capture_buf[log_capture_pos] = '\0';
}

enum { FSM_ST_IDLE, FSM_ST_RUNNING, FSM_ST_ERROR };
enum { FSM_EV_START, FSM_EV_STOP, FSM_EV_FAULT };

static int fsm_action_called = 0;
static void fsm_on_start(void *ctx) { (void)ctx; fsm_action_called = 1; }
static void fsm_on_stop(void *ctx)  { (void)ctx; fsm_action_called = 2; }

static bool fsm_guard_deny(void *ctx) { (void)ctx; return false; }

static const SYN_FSM_Transition test_fsm_table[] = {
    { FSM_ST_IDLE,    FSM_EV_START, FSM_ST_RUNNING, NULL,           fsm_on_start },
    { FSM_ST_RUNNING, FSM_EV_STOP,  FSM_ST_IDLE,    NULL,           fsm_on_stop  },
    { FSM_ST_RUNNING, FSM_EV_FAULT, FSM_ST_ERROR,   NULL,           NULL         },
    { FSM_ST_IDLE,    FSM_EV_FAULT, FSM_ST_IDLE,    fsm_guard_deny, NULL         },
    SYN_FSM_END
};

static void test_fsm(void)
{

    /* Re-init log so FSM can log transitions */
    log_capture_pos = 0;
    syn_log_init(log_capture_output, SYN_LOG_DEBUG);

    SYN_FSM fsm;
    syn_fsm_init(&fsm, test_fsm_table, FSM_ST_IDLE, "fsm");

    TEST_ASSERT_EQUAL(FSM_ST_IDLE, syn_fsm_state(&fsm));
    TEST_ASSERT_TRUE(syn_fsm_in_state(&fsm, FSM_ST_IDLE));

    /* Dispatch START */
    fsm_action_called = 0;
    bool took = syn_fsm_dispatch(&fsm, FSM_EV_START);
    TEST_ASSERT_TRUE(took);
    TEST_ASSERT_EQUAL(FSM_ST_RUNNING, syn_fsm_state(&fsm));
    TEST_ASSERT_EQUAL_INT(1, fsm_action_called);

    /* Dispatch STOP */
    took = syn_fsm_dispatch(&fsm, FSM_EV_STOP);
    TEST_ASSERT_TRUE(took);
    TEST_ASSERT_EQUAL(FSM_ST_IDLE, syn_fsm_state(&fsm));
    TEST_ASSERT_EQUAL_INT(2, fsm_action_called);

    /* Guard blocks transition */
    took = syn_fsm_dispatch(&fsm, FSM_EV_FAULT);
    TEST_ASSERT_FALSE(took);
    TEST_ASSERT_EQUAL(FSM_ST_IDLE, syn_fsm_state(&fsm));

    /* No matching transition */
    took = syn_fsm_dispatch(&fsm, FSM_EV_STOP);
    TEST_ASSERT_FALSE(took);

    /* Force state */
    syn_fsm_set_state(&fsm, FSM_ST_ERROR);
    TEST_ASSERT_EQUAL(FSM_ST_ERROR, syn_fsm_state(&fsm));
}

void run_fsm_tests(void)
{
    RUN_TEST(test_fsm);
}
