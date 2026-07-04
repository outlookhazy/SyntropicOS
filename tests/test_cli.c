/**
 * @file test_cli.c
 * @brief Unity tests for syn_cli — full coverage.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/cli/syn_cli.h"
#include "syntropic/system/syn_errlog.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"

#include <string.h>

/* ── Shared output capture ─────────────────────────────────────────────── */
/* CLI now writes directly to syn_port_serial_write, which is mocked
 * by mock_port.c into mock_serial_tx_buf. These macros alias the mock
 * state so existing test assertions continue to work unchanged. */

#define cli_output_buf  ((char *)mock_serial_tx_buf)
#define cli_output_pos  mock_serial_tx_len

static void clear_output(void)
{
    mock_serial_tx_len = 0;
    mock_serial_tx_buf[0] = '\0';
}

/* ── Command handlers ───────────────────────────────────────────────────── */

static int led_handler_called = 0;
static int led_handler_argc = 0;
static char led_handler_arg1[32];

static int cmd_led(int argc, char *argv[])
{
    led_handler_called = 1;
    led_handler_argc = argc;
    if (argc > 1) {
        strncpy(led_handler_arg1, argv[1], sizeof(led_handler_arg1) - 1);
        led_handler_arg1[sizeof(led_handler_arg1) - 1] = '\0';
    }
    return 0;
}

static int cmd_status(int argc, char *argv[])
{
    (void)argc; (void)argv;
    return 0;
}

static int cmd_fail(int argc, char *argv[])
{
    (void)argc; (void)argv;
    return -1; /* Non-zero return triggers error output */
}

static const SYN_CLI_Command test_commands[] = {
    { "led",    "led <on|off>  - Control LED",  cmd_led    },
    { "status", "status        - Show status",  cmd_status },
    { "fail",   "fail          - Returns error", cmd_fail  },
    { "longnm", NULL,                            cmd_status }, /* no help */
};

/* ── Test: basic command dispatch and help ─────────────────────────────── */

static void test_cli_basic(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* led command */
    led_handler_called = 0;
    led_handler_argc = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led on");
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_INT(2, led_handler_argc);
    TEST_ASSERT_EQUAL_STRING("on", led_handler_arg1);

    /* Unknown command */
    clear_output();
    syn_cli_process_line(&cli, "bogus");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));

    /* help command */
    clear_output();
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "led"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "status"));

    /* '?' also triggers help */
    clear_output();
    syn_cli_process_line(&cli, "?");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
}

/* ── Test: string output via serial port ─────────────────────────────── */

static void test_cli_puts_fn(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* CLI now writes directly to syn_port_serial_write.
     * Verify output works for both command dispatch and error paths. */
    clear_output();
    syn_cli_process_line(&cli, "led on");
    /* Just verifying no crash; serial output was exercised */

    clear_output();
    syn_cli_process_line(&cli, "bogus");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));
}

/* ── Test: puts_fn with NULL str ──────────────────────────────────────── */

static void test_cli_puts_null_str(void)
{
    SYN_CLI cli;
    /* putchar_fn only, no puts_fn — fallback to char-by-char */
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    /* Process a command that triggers output via putchar fallback */
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
}

/* ── Test: process_char with echo, backspace, Ctrl-C ──────────────────── */

static void test_cli_process_char_echo(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    clear_output();

    syn_cli_set_echo(&cli, true);
    syn_cli_print_prompt(&cli);

    /* Type "led off\r" */
    const char *input = "led off\r";
    while (*input) {
        syn_cli_process_char(&cli, *input++);
    }
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("off", led_handler_arg1);

    /* Backspace: type "lex\b" then "d off\r" → "led off" */
    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    {
        const char bs_del = '\b'; /* 0x08 backspace */
        const char *bs1 = "lex"; /* type 3 chars */
        for (const char *p = bs1; *p; p++) syn_cli_process_char(&cli, *p);
        syn_cli_process_char(&cli, bs_del); /* erase 'x' → "le" */
        const char *bs2 = "d off\r"; /* → "led off" */
        for (const char *p = bs2; *p; p++) syn_cli_process_char(&cli, *p);
    }
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("off", led_handler_arg1);

    /* Enter on empty line → no handler called */
    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);

    /* Ctrl-C cancels line */
    led_handler_called = 0;
    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 0x03); /* Ctrl-C */
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: process_char without echo (backspace, Ctrl-C, Ctrl-U paths) ── */

static void test_cli_process_char_no_echo(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");
    syn_cli_set_echo(&cli, false);

    /* Backspace with no-echo: type "led\bd\n" → executes "led" */
    /* l+e+d = "led", \b erases 'd' → "le", re-type 'd' → "led" */
    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 'd');
    syn_cli_process_char(&cli, '\b'); /* backspace — erase 'd' → "le" */
    syn_cli_process_char(&cli, 'd'); /* re-type → "led" */

    led_handler_called = 0;
    syn_cli_process_char(&cli, '\n'); /* \n also triggers execute */
    TEST_ASSERT_EQUAL_INT(1, led_handler_called); /* "led" command with argc=1 */

    /* Ctrl-C with no-echo */
    led_handler_called = 0;
    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 0x03); /* Ctrl-C clears line */
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: Ctrl-U (clear line) ─────────────────────────────────────────── */

static void test_cli_ctrl_u(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");
    syn_cli_set_echo(&cli, true);

    /* Type some chars then Ctrl-U to clear */
    syn_cli_process_char(&cli, 'b');
    syn_cli_process_char(&cli, 'o');
    syn_cli_process_char(&cli, 'g');
    syn_cli_process_char(&cli, 0x15); /* Ctrl-U */

    /* Should now dispatch empty line (no handler) */
    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);

    /* Ctrl-U with no-echo flag */
    syn_cli_set_echo(&cli, false);
    syn_cli_process_char(&cli, 'b');
    syn_cli_process_char(&cli, 'o');
    syn_cli_process_char(&cli, 0x15); /* Ctrl-U no-echo */
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: ANSI escape sequences ──────────────────────────────────────── */

static void test_cli_escape_sequences(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* ESC [ A = Up Arrow → maps to Ctrl-P (history recall) */
    /* With empty history, should just return */
    syn_cli_process_char(&cli, 0x1B); /* ESC */
    syn_cli_process_char(&cli, '[');  /* bracket */
    syn_cli_process_char(&cli, 'A'); /* Up arrow */
    /* No crash = pass */

    /* ESC [ B = Down Arrow → ignored (other arrow) */
    syn_cli_process_char(&cli, 0x1B);
    syn_cli_process_char(&cli, '[');
    syn_cli_process_char(&cli, 'B'); /* Down arrow — should be ignored */

    /* ESC followed by non-bracket → resets state */
    syn_cli_process_char(&cli, 0x1B);
    syn_cli_process_char(&cli, 'X'); /* not '[' — resets escape state */

    /* After escape reset, normal char 'led\r' should work */
    led_handler_called = 0;
    const char *cmd = "led\r";
    while (*cmd) syn_cli_process_char(&cli, *cmd++);
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
}

/* ── Test: Tab character → treated as space ──────────────────────────── */

static void test_cli_tab_as_space(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* "led\ton\r" — tab between command and arg */
    led_handler_called = 0;
    led_handler_argc = 0;
    led_handler_arg1[0] = '\0';

    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 'd');
    syn_cli_process_char(&cli, '\t'); /* tab → space */
    syn_cli_process_char(&cli, 'o');
    syn_cli_process_char(&cli, 'n');
    syn_cli_process_char(&cli, '\r');

    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_INT(2, led_handler_argc);
    TEST_ASSERT_EQUAL_STRING("on", led_handler_arg1);
}

/* ── Test: Ignore other control chars (< 0x20, not tab/CR/LF) ─────────── */

static void test_cli_ignore_control_chars(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* Ctrl-D (0x04), Ctrl-E, etc. should be silently ignored */
    syn_cli_process_char(&cli, 0x04); /* Ctrl-D */
    syn_cli_process_char(&cli, 0x05); /* Ctrl-E */
    syn_cli_process_char(&cli, 0x06); /* Ctrl-F */

    /* After ignoring them, normal command should still work */
    led_handler_called = 0;
    const char *cmd = "led\r";
    while (*cmd) syn_cli_process_char(&cli, *cmd++);
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
}

/* ── Test: Command returns non-zero error code ─────────────────────────── */

static void test_cli_command_error_return(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    syn_cli_process_line(&cli, "fail");
    /* Should print "Error: -1\r\n" */
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Error:"));
}

/* ── Test: Quoted string argument ─────────────────────────────────────── */

static void test_cli_quoted_args(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led \"hello world\"");
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("hello world", led_handler_arg1);
}

/* ── Test: syn_cli_printf ─────────────────────────────────────────────── */

static void test_cli_printf(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    syn_cli_printf(&cli, "value=%d\r\n", 99);
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "value=99"));
}

/* ── Test: Buffer full — silently drop characters ─────────────────────── */

static void test_cli_buffer_full(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* Fill the buffer to capacity */
    for (int i = 0; i < SYN_CLI_LINE_BUF_SIZE + 10; i++) {
        syn_cli_process_char(&cli, 'a');
    }
    /* Should not crash; excess chars silently dropped */
    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    /* "aaa...a" is unknown command, not led */
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: Backspace on empty buffer (no underflow) ───────────────────── */

static void test_cli_backspace_empty(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* Backspace on empty line should be a no-op */
    syn_cli_process_char(&cli, '\b');
    syn_cli_process_char(&cli, 0x7F);
    /* No crash = pass */
    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: Help with command that has no help string ──────────────────── */

static void test_cli_help_no_help_string(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    syn_cli_process_line(&cli, "help");
    /* longnm has NULL help — should still appear without "— " description */
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "longnm"));
}

/* ── Test: Builtin 'version' command ──────────────────────────────────── */

static void test_cli_builtin_version(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    syn_cli_process_line(&cli, "version");
    /* Should print version info — just check it produced something */
    TEST_ASSERT_TRUE(cli_output_pos > 0);
}

/* ── Test: Builtin 'uptime' command ───────────────────────────────────── */

static void test_cli_builtin_uptime(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    clear_output();
    syn_cli_process_line(&cli, "uptime");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Uptime:"));
}

/* ── Test: Builtin 'errors' command — no errlog configured ───────────── */

static void test_cli_builtin_errors_no_log(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* Explicitly set errlog to NULL (default is already NULL) */
    syn_cli_set_errlog(NULL);

    clear_output();
    syn_cli_process_line(&cli, "errors");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "No error log configured"));
}

/* ── Test: Builtin 'errors' command — empty log ──────────────────────── */

static void test_cli_builtin_errors_empty(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 0);
    syn_cli_set_errlog(&elog);

    clear_output();
    syn_cli_process_line(&cli, "errors");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Error log:"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "0 stored"));

    syn_cli_set_errlog(NULL); /* clean up global */
}

/* ── Test: Builtin 'errors' command — with entries ───────────────────── */

static void test_cli_builtin_errors_with_entries(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 1);
    syn_errlog_record(&elog, 0x0042, SYN_ERR_WARNING, 100);
    syn_errlog_record(&elog, 0xDEAD, SYN_ERR_ERROR,   200);
    syn_errlog_record(&elog, 0xBEEF, SYN_ERR_FATAL,   300);
    syn_cli_set_errlog(&elog);

    clear_output();
    syn_cli_process_line(&cli, "errors");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Error log:"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "code=0x"));

    syn_cli_set_errlog(NULL);
}

/* ── Test: Builtin 'tasks' command — no scheduler configured ─────────── */

static void test_cli_builtin_tasks_no_sched(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    syn_cli_set_scheduler(NULL);

    clear_output();
    syn_cli_process_line(&cli, "tasks");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "No scheduler configured"));
}

/* ── Test: Builtin 'tasks' command — with tasks ──────────────────────── */

static SYN_PT_Status dummy_task_fn(SYN_PT *pt, struct SYN_Task *task)
{
    (void)pt; (void)task;
    return PT_ENDED;
}

static void test_cli_builtin_tasks_with_sched(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_Task tasks[3];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "blink",  dummy_task_fn, 0, NULL);
    syn_task_create(&tasks[1], "serial", dummy_task_fn, 1, NULL);
    syn_task_create(&tasks[2], NULL,     dummy_task_fn, 2, NULL); /* unnamed */
    tasks[2].state = SYN_TASK_SUSPENDED;

    syn_sched_init(&sched, tasks, 3);
    syn_cli_set_scheduler(&sched);

    clear_output();
    syn_cli_process_line(&cli, "tasks");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Tasks:"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "blink"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "(unnamed)"));

    syn_cli_set_scheduler(NULL);
}

/* ── Test: History — push, recall with Ctrl-P ────────────────────────── */

#if SYN_CLI_HISTORY_DEPTH > 0
static void test_cli_history(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* Execute some commands to build history */
    syn_cli_process_line(&cli, "led on");
    syn_cli_process_line(&cli, "status");

    /* Ctrl-P recalls previous command ("status") */
    clear_output();
    syn_cli_process_char(&cli, 0x10); /* Ctrl-P (Up arrow) */

    /* Execute recalled command — should run "status" */
    status_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(1, status_handler_called);

    /* Duplicate suppression: re-running same command should not duplicate */
    syn_cli_process_line(&cli, "led on");
    syn_cli_process_line(&cli, "led on"); /* duplicate, should be suppressed */

    /* Recall: empty history count = 0, Ctrl-P is no-op */
    SYN_CLI cli2;
    syn_cli_init(&cli2, test_commands, 4, "> ");
    syn_cli_process_char(&cli2, 0x10); /* Ctrl-P with empty history — no-op */

    /* History via ANSI Up Arrow sequence → ESC [ A */
    led_handler_called = 0;
    syn_cli_process_char(&cli, 0x1B);
    syn_cli_process_char(&cli, '[');
    syn_cli_process_char(&cli, 'A'); /* Maps to Ctrl-P */
    syn_cli_process_char(&cli, '\r');
    /* Should have recalled and executed a command from history */
    TEST_ASSERT_TRUE(led_handler_called > 0 || status_handler_called > 1);
}
#endif

/* ── Test: process_line with NULL/empty line ─────────────────────────── */

static void test_cli_empty_line(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    led_handler_called = 0;
    syn_cli_process_line(&cli, "");
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);

    syn_cli_process_line(&cli, "   "); /* only whitespace */
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: Help for command with long name (padding path) ─────────────── */

static void test_cli_help_padding(void)
{
    SYN_CLI cli;
    /* Use a command with a short name to exercise the padding loop */
    static const SYN_CLI_Command cmds[] = {
        { "go", "go - short name", cmd_status },
    };
    syn_cli_init(&cli, cmds, 1, "> ");

    clear_output();
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "go"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "--"));
}

/* ── Test: Command with NULL handler (no-crash) ───────────────────────── */

static void test_cli_null_handler(void)
{
    static const SYN_CLI_Command cmds[] = {
        { "noop", "noop - does nothing", NULL },
    };
    SYN_CLI cli;
    syn_cli_init(&cli, cmds, 1, "> ");

    /* Should dispatch without crash even with NULL handler */
    led_handler_called = 0;
    syn_cli_process_line(&cli, "noop");
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: Errors command — severity out of range (????) ─────────────── */

static void test_cli_errors_unknown_severity(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_ErrEntry entries[2];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 2, 0);

    /* Manually craft an entry with out-of-range severity */
    syn_errlog_record(&elog, 0x0001, SYN_ERR_INFO, 0);
    /* Directly overwrite severity to an invalid value */
    entries[0].severity = 99;

    syn_cli_set_errlog(&elog);

    clear_output();
    syn_cli_process_line(&cli, "errors");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "????"));

    syn_cli_set_errlog(NULL);
}

/* ── Test: Tasks command — task state out of range (???) ─────────────── */

static void test_cli_tasks_unknown_state(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_Task tasks[1];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "weird", dummy_task_fn, 0, NULL);
    tasks[0].state = 99; /* invalid state → "  ???" */

    syn_sched_init(&sched, tasks, 1);
    syn_cli_set_scheduler(&sched);

    clear_output();
    syn_cli_process_line(&cli, "tasks");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "???"));

    syn_cli_set_scheduler(NULL);
}

/* ── Test runner ─────────────────────────────────────────────────────── */

void run_cli_tests(void)
{
    RUN_TEST(test_cli_basic);
    RUN_TEST(test_cli_puts_fn);
    RUN_TEST(test_cli_puts_null_str);
    RUN_TEST(test_cli_process_char_echo);
    RUN_TEST(test_cli_process_char_no_echo);
    RUN_TEST(test_cli_ctrl_u);
    RUN_TEST(test_cli_escape_sequences);
    RUN_TEST(test_cli_tab_as_space);
    RUN_TEST(test_cli_ignore_control_chars);
    RUN_TEST(test_cli_command_error_return);
    RUN_TEST(test_cli_quoted_args);
    RUN_TEST(test_cli_printf);
    RUN_TEST(test_cli_buffer_full);
    RUN_TEST(test_cli_backspace_empty);
    RUN_TEST(test_cli_help_no_help_string);
    RUN_TEST(test_cli_builtin_version);
    RUN_TEST(test_cli_builtin_uptime);
    RUN_TEST(test_cli_builtin_errors_no_log);
    RUN_TEST(test_cli_builtin_errors_empty);
    RUN_TEST(test_cli_builtin_errors_with_entries);
    RUN_TEST(test_cli_builtin_tasks_no_sched);
    RUN_TEST(test_cli_builtin_tasks_with_sched);
#if SYN_CLI_HISTORY_DEPTH > 0
    RUN_TEST(test_cli_history);
#endif
    RUN_TEST(test_cli_empty_line);
    RUN_TEST(test_cli_help_padding);
    RUN_TEST(test_cli_null_handler);
    RUN_TEST(test_cli_errors_unknown_severity);
    RUN_TEST(test_cli_tasks_unknown_state);
}
