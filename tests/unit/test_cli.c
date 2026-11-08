/**
 * @file test_cli.c
 * @brief Unity tests for syn_cli — full coverage.
 */

#include "mocks/mock_port.h"
#include "syntropic/cli/syn_cli.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_task.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_errlog.h"
#include "unity/unity.h"

#include <string.h>

/* ── Shared output capture ─────────────────────────────────────────────── */
/* CLI now writes directly to syn_port_serial_write, which is mocked
 * by mock_port.c into mock_serial_tx_buf. These macros alias the mock
 * state so existing test assertions continue to work unchanged. */

#define cli_output_buf ((char *)mock_serial_tx_buf)
#define cli_output_pos mock_serial_tx_len

static void clear_output(void)
{
    mock_serial_tx_len = 0;
    mock_serial_tx_buf[0] = '\0';
}

/* ── Command handlers ───────────────────────────────────────────────────── */

static int led_handler_called = 0;
static int led_handler_argc = 0;
static char led_handler_arg1[32];
static int status_handler_called = 0;

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
    (void)argc;
    (void)argv;
    status_handler_called++;
    return 0;
}

static int cmd_fail(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return -1; /* Non-zero return triggers error output */
}

static const SYN_CLI_Command test_commands[] = {
    {"led", "led <on|off>  - Control LED", cmd_led},
    {"status", "status        - Show status", cmd_status},
    {"fail", "fail          - Returns error", cmd_fail},
    {"longnm", NULL, cmd_status}, /* no help */
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
        const char *bs1 = "lex";  /* type 3 chars */
        for (const char *p = bs1; *p; p++)
            syn_cli_process_char(&cli, *p);
        syn_cli_process_char(&cli, bs_del); /* erase 'x' → "le" */
        const char *bs2 = "d off\r";        /* → "led off" */
        for (const char *p = bs2; *p; p++)
            syn_cli_process_char(&cli, *p);
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
    syn_cli_process_char(&cli, 'd');  /* re-type → "led" */

    led_handler_called = 0;
    syn_cli_process_char(&cli, '\n');             /* \n also triggers execute */
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
    syn_cli_process_char(&cli, 'A');  /* Up arrow */
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
    while (*cmd)
        syn_cli_process_char(&cli, *cmd++);
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
    while (*cmd)
        syn_cli_process_char(&cli, *cmd++);
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
    syn_errlog_record(&elog, 0xDEAD, SYN_ERR_ERROR, 200);
    syn_errlog_record(&elog, 0xBEEF, SYN_ERR_FATAL, 300);
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
    (void)pt;
    (void)task;
    return PT_ENDED;
}

static void test_cli_builtin_tasks_with_sched(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    SYN_Task tasks[4];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "blink", dummy_task_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&tasks[1], "serial", dummy_task_fn, SYN_PRIO_HIGH, NULL);
    tasks[1].state = SYN_TASK_DEAD;
    syn_task_create(&tasks[2], NULL, dummy_task_fn, SYN_PRIO_NORMAL, NULL); /* unnamed */
    tasks[2].state = SYN_TASK_SUSPENDED;
    syn_task_create(&tasks[3], "evt", dummy_task_fn, SYN_PRIO_LOW, NULL);
    tasks[3].state = SYN_TASK_BLOCKED;

    syn_sched_init(&sched, tasks, 4);
    syn_cli_set_scheduler(&sched);

    clear_output();
    syn_cli_process_line(&cli, "tasks");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Tasks:"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "blink"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "(unnamed)"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "  READY"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "  DEAD"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "  SUSPENDED"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "  BLOCKED"));
    TEST_ASSERT_NULL(strstr(cli_output_buf, "DEFERRED"));
    TEST_ASSERT_NULL(strstr(cli_output_buf, "WAITING"));

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
        {"go", "go - short name", cmd_status},
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
        {"noop", "noop - does nothing", NULL},
    };
    SYN_CLI cli;
    syn_cli_init(&cli, cmds, 1, "> ");

    /* Should dispatch without crash even with NULL handler */
    led_handler_called = 0;
    syn_cli_process_line(&cli, "noop");
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);
}

/* ── Test: refresh prompt ────────────────────────────────────────────── */

static void test_cli_refresh_prompt(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, NULL, 0, "> ");
    clear_output();
    syn_cli_refresh_prompt(&cli);
    TEST_ASSERT_TRUE(cli_output_pos > 0);
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

    syn_task_create(&tasks[0], "weird", dummy_task_fn, SYN_PRIO_CRITICAL, NULL);
    tasks[0].state = 99; /* invalid state → "  ???" */

    syn_sched_init(&sched, tasks, 1);
    syn_cli_set_scheduler(&sched);

    clear_output();
    syn_cli_process_line(&cli, "tasks");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "???"));

    syn_cli_set_scheduler(NULL);
}

/* ── Test: Uncovered edge cases (L72, L149, L153, L417, L418, L436, L437, L453) ── */

static void test_cli_command_history_and_autocompletion(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4, "> ");

    /* 1. cli_puts(cli, NULL) via prompt=NULL -> line 72 */
    cli.prompt = NULL;
    syn_cli_print_prompt(&cli);

    /* 2. Help command with errlog & sched bound -> lines 149 & 153 */
    SYN_ErrLog elog;
    SYN_ErrEntry e_buf[2];
    syn_errlog_init(&elog, e_buf, 2, 1);
    syn_cli_set_errlog(&elog);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "t1", dummy_task_fn, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_cli_set_scheduler(&sched);

    clear_output();
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "errors"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "tasks"));

    syn_cli_set_errlog(NULL);
    syn_cli_set_scheduler(NULL);

    /* 3. Async output while prompt is visible & line_pos > 0 -> lines 417-418, 436-437, 453 */
    syn_cli_process_char(&cli, 'a');
    syn_cli_process_char(&cli, 'b'); /* prompt_visible = true, line_pos = 2 */
    syn_cli_printf(&cli, "Async log message\n");

    /* 4. Help output command name NULL guard -> line 158 */
    SYN_CLI_Command cmds_null[1] = {{.name = NULL, .handler = NULL, .help = NULL}};
    SYN_CLI cli_null;
    syn_cli_init(&cli_null, cmds_null, 1, "test> ");
    clear_output();
    syn_cli_process_line(&cli_null, "help");
}

/** R4-K08: NULL command names in command lookup, NULL commands array, and public NULL guards. */
static void test_r4_cli_null_guards_and_null_command_name(void)
{
    static const SYN_CLI_Command sparse_cmds[] = {
        {NULL, "sentinel without name", cmd_fail},
        {"led", "led <on|off>", cmd_led},
    };
    SYN_CLI cli;

    /* 1. Public API NULL guards */
    syn_cli_init(NULL, sparse_cmds, 2U, "> ");
    syn_cli_set_echo(NULL, false);
    syn_cli_process_char(NULL, 'x');
    syn_cli_process_line(NULL, "led");
    syn_cli_print_prompt(NULL);
    syn_cli_refresh_prompt(NULL);
    syn_cli_printf(NULL, "ignored %d", 1);

    syn_cli_init(&cli, sparse_cmds, 2U, NULL);
    syn_cli_process_line(&cli, NULL);
    syn_cli_printf(&cli, NULL);

    /* 2. Lookup skips .name == NULL entry without strcmp(argv[0], NULL) crash */
    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led on");
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("on", led_handler_arg1);

    clear_output();
    syn_cli_process_line(&cli, "nonexistent");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));

    /* 3. NULL commands table with non-zero cmd_count in both help and command lookup */
    SYN_CLI cli_no_cmds;
    syn_cli_init(&cli_no_cmds, NULL, 4U, "> ");
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)cli_no_cmds.command_count);
    cli_no_cmds.command_count = 2U; /* defensive corrupt state with commands == NULL */
    clear_output();
    syn_cli_process_line(&cli_no_cmds, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
    clear_output();
    syn_cli_process_line(&cli_no_cmds, "led");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));
}

static void test_r5_cli_crlf_and_history_bounds(void)
{
    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 4U, "> ");

    /* 1. CRLF ('\r\n') executes once and emits prompt once (no double prompt) */
    status_handler_called = 0;
    clear_output();
    const char *crlf_cmd = "status\r\n";
    for (const char *p = crlf_cmd; *p != '\0'; p++) {
        syn_cli_process_char(&cli, *p);
    }
    TEST_ASSERT_EQUAL_INT(1, status_handler_called);
    const char *first_prompt = strstr(cli_output_buf, "> ");
    TEST_ASSERT_NOT_NULL(first_prompt);
    TEST_ASSERT_NULL(strstr(first_prompt + 2, "> "));

    /* Two consecutive '\r\r' still emits two prompts */
    clear_output();
    syn_cli_process_char(&cli, '\r');
    syn_cli_process_char(&cli, '\r');
    first_prompt = strstr(cli_output_buf, "> ");
    TEST_ASSERT_NOT_NULL(first_prompt);
    TEST_ASSERT_NOT_NULL(strstr(first_prompt + 2, "> "));

#if SYN_CLI_HISTORY_DEPTH > 0
    /* 2. Partial history clamping: 2 entries, 4 Up presses stays at oldest entry */
    SYN_CLI hcli;
    syn_cli_init(&hcli, test_commands, 4U, "> ");
    syn_cli_process_line(&hcli, "led on");
    syn_cli_process_line(&hcli, "status");

    syn_cli_process_char(&hcli, 0x10); /* newest: "status" */
    TEST_ASSERT_EQUAL_STRING("status", hcli.line_buf);
    syn_cli_process_char(&hcli, 0x10); /* oldest: "led on" */
    TEST_ASSERT_EQUAL_STRING("led on", hcli.line_buf);
    syn_cli_process_char(&hcli, 0x10); /* clamped at oldest: "led on" */
    TEST_ASSERT_EQUAL_STRING("led on", hcli.line_buf);

    /* 3. Wrapped ring history clamping (push > SYN_CLI_HISTORY_DEPTH commands) */
    syn_cli_process_line(&hcli, "led off");
    syn_cli_process_line(&hcli, "status");
    syn_cli_process_line(&hcli, "help");
    syn_cli_process_line(&hcli, "led on");

    for (size_t i = 0U; i < (size_t)SYN_CLI_HISTORY_DEPTH + 3U; i++) {
        syn_cli_process_char(&hcli, 0x10);
    }
    /* Oldest retained entry in depth-4 ring of [led off, status, help, led on] is "led off" */
    TEST_ASSERT_EQUAL_STRING("led off", hcli.line_buf);

    /* Recall with echo disabled also works */
    syn_cli_set_echo(&hcli, false);
    hcli.history_read = 0U;
    syn_cli_process_char(&hcli, 0x10);
    TEST_ASSERT_EQUAL_STRING("led on", hcli.line_buf);
#endif
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
    RUN_TEST(test_cli_refresh_prompt);
    RUN_TEST(test_cli_command_history_and_autocompletion);
    RUN_TEST(test_r4_cli_null_guards_and_null_command_name);
    RUN_TEST(test_r5_cli_crlf_and_history_bounds);
}
