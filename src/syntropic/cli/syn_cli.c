#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_CLI) || SYN_USE_CLI

#if defined(SYN_USE_FMT) && !SYN_USE_FMT
#error "syn_cli requires SYN_USE_FMT=1"
#endif

/**
 * @file syn_cli.c
 * @brief Command-line interpreter implementation.
 */

#include "../port/syn_port_serial.h"
#include "../util/syn_fmt.h"
#include "syn_cli.h"

#include <string.h>

#if !defined(SYN_CLI_USE_PRINTF) || SYN_CLI_USE_PRINTF
#include <stdarg.h>
#include <stdio.h>
#endif

/* ── Internal helpers ───────────────────────────────────────────────────── */

/* Forward declarations for built-in commands */
#if SYN_CLI_CMD_VERSION
static void cli_builtin_version(const SYN_CLI *cli);
#endif
#if SYN_CLI_CMD_UPTIME
static void cli_builtin_uptime(const SYN_CLI *cli);
#endif
#if SYN_CLI_CMD_ERRORS
static void cli_builtin_errors(const SYN_CLI *cli);
#endif
#if SYN_CLI_CMD_TASKS
static void cli_builtin_tasks(const SYN_CLI *cli);
#endif

/* Runtime-configured singletons (set via syn_cli_set_*) */
#if SYN_CLI_CMD_ERRORS
struct SYN_ErrLog;
/** @brief Global error log singleton reference for CLI diagnostics. */
static struct SYN_ErrLog *s_cli_errlog = NULL;
#endif
#if SYN_CLI_CMD_TASKS
struct SYN_Sched;
/** @brief Global scheduler singleton reference for CLI task listing. */
static struct SYN_Sched *s_cli_sched = NULL;
#endif

/**
 * @brief Emit a single character via the console serial port.
 * @param cli  CLI instance (unused, kept for internal API consistency).
 * @param ch   Character to emit.
 */
static void cli_putchar(const SYN_CLI *cli, char ch)
{
    (void)cli;
    syn_port_serial_write_byte((uint8_t)ch);
}

/**
 * @brief Emit a null-terminated string via the console serial port.
 * @param cli  CLI instance (unused, kept for internal API consistency).
 * @param str  String to print.
 */
static void cli_puts(const SYN_CLI *cli, const char *str)
{
    (void)cli;
    if (str == NULL) {
        return;
    }
    size_t len = strlen(str);
    if (len > 0U) {
        syn_port_serial_write((const uint8_t *)str, len);
    }
}

/**
 * @brief Tokenize a line buffer in-place into argv[].
 *
 * Handles double-quoted strings (quotes are stripped).
 *
 * @param line      Input line (modified in-place).
 * @param argv      [out] Array of token pointers.
 * @param max_args  Maximum number of tokens.
 * @return Token count (argc).
 */
static int cli_tokenize(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p && argc < max_args) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        /* Handle quoted string */
        if (*p == '"') {
            p++; /* skip opening quote */
            argv[argc++] = p;
            while (*p && *p != '"') {
                p++;
            }
            if (*p == '"') {
                *p++ = '\0';
            }
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') {
                p++;
            }
            if (*p) {
                *p++ = '\0';
            }
        }
    }

    return argc;
}

/**
 * @brief Print built-in and user-registered command help table.
 * @param cli CLI instance.
 */
static void cli_print_help(const SYN_CLI *cli)
{
    cli_puts(cli, "Available commands:\r\n");
    cli_puts(cli, "  help           -- Show this help\r\n");
#if SYN_CLI_CMD_VERSION
    cli_puts(cli, "  version        -- Show build info\r\n");
#endif
#if SYN_CLI_CMD_UPTIME
    cli_puts(cli, "  uptime         -- Show system uptime\r\n");
#endif
#if SYN_CLI_CMD_ERRORS
    if (s_cli_errlog != NULL) {
        cli_puts(cli, "  errors         -- Dump error log\r\n");
    }
#endif
#if SYN_CLI_CMD_TASKS
    if (s_cli_sched != NULL) {
        cli_puts(cli, "  tasks          -- Show scheduler tasks\r\n");
    }
#endif

    if (cli->commands == NULL) {
        return;
    }

    for (size_t i = 0U; i < cli->command_count; i++) {
        if (cli->commands[i].name == NULL) {
            continue;
        }
        cli_puts(cli, "  ");
        cli_puts(cli, cli->commands[i].name);
        if (cli->commands[i].help != NULL) {
            /* Pad to align with built-in entries (15 chars for name) */
            size_t name_len = strlen(cli->commands[i].name);
            for (size_t p = name_len; p < 15U; p++) {
                cli_putchar(cli, ' ');
            }
            cli_puts(cli, "-- ");
            cli_puts(cli, cli->commands[i].help);
        }
        cli_puts(cli, "\r\n");
    }
}

/**
 * @brief Search the user command table and execute a matching command.
 * @param cli  CLI instance.
 * @param argc Token count.
 * @param argv Token vector.
 * @return true if a matching command entry was found, false otherwise.
 */
static bool cli_exec_user_command(SYN_CLI *cli, int argc, char *argv[])
{
    if (cli->commands == NULL) {
        return false;
    }

    for (size_t i = 0U; i < cli->command_count; i++) {
        if (cli->commands[i].name == NULL) {
            continue;
        }
        if (strcmp(argv[0], cli->commands[i].name) == 0) {
            if (cli->commands[i].handler != NULL) {
                int ret = cli->commands[i].handler(argc, argv);
                if (ret != 0) {
                    char buf[32];
                    const char *parts[] = {"Error: ", NULL, "\r\n"};
                    char num[12];
                    syn_fmt_int(num, sizeof(num), ret);
                    parts[1] = num;
                    syn_fmt_concat(buf, sizeof(buf), parts, 3);
                    cli_puts(cli, buf);
                }
            }
            return true;
        }
    }
    return false;
}

/**
 * @brief Look up and execute a command.
 * @param cli   CLI instance.
 * @param line  Input line (modified in-place during tokenization).
 */
static void cli_dispatch(SYN_CLI *cli, char *line)
{
    char *argv[SYN_CLI_MAX_ARGS];
    int argc = cli_tokenize(line, argv, SYN_CLI_MAX_ARGS);

    if (argc == 0) {
        return; /* empty line */
    }

    /* Built-in: help */
    if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "?") == 0) {
        cli_print_help(cli);
        return;
    }

    /* Built-in: version */
#if SYN_CLI_CMD_VERSION
    if (strcmp(argv[0], "version") == 0) {
        cli_builtin_version(cli);
        return;
    }
#endif

    /* Built-in: uptime */
#if SYN_CLI_CMD_UPTIME
    if (strcmp(argv[0], "uptime") == 0) {
        cli_builtin_uptime(cli);
        return;
    }
#endif

    /* Built-in: errors */
#if SYN_CLI_CMD_ERRORS
    if (strcmp(argv[0], "errors") == 0) {
        cli_builtin_errors(cli);
        return;
    }
#endif

    /* Built-in: tasks */
#if SYN_CLI_CMD_TASKS
    if (strcmp(argv[0], "tasks") == 0) {
        cli_builtin_tasks(cli);
        return;
    }
#endif

    if (cli_exec_user_command(cli, argc, argv)) {
        return;
    }

    /* Command not found */
    cli_puts(cli, "Unknown command: ");
    cli_puts(cli, argv[0]);
    cli_puts(cli, "\r\nType 'help' for a list of commands.\r\n");
}

#if SYN_CLI_HISTORY_DEPTH > 0
/**
 * @brief Push a non-empty command line into the history ring buffer.
 * @param cli  CLI instance.
 * @param line Null-terminated line string.
 */
static void cli_history_push(SYN_CLI *cli, const char *line)
{
    const char *p = line;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p == '\0') {
        cli->history_read = 0U;
        return;
    }

    /* Don't store duplicates of the last entry */
    if (cli->history_count > 0U) {
        size_t last = (cli->history_write + SYN_CLI_HISTORY_DEPTH - 1U) % SYN_CLI_HISTORY_DEPTH;
        if (strcmp(cli->history[last], line) == 0) {
            cli->history_read = 0U;
            return;
        }
    }

    strncpy(cli->history[cli->history_write], line, SYN_CLI_LINE_BUF_SIZE - 1U);
    cli->history[cli->history_write][SYN_CLI_LINE_BUF_SIZE - 1U] = '\0';

    cli->history_write = (cli->history_write + 1U) % SYN_CLI_HISTORY_DEPTH;
    if (cli->history_count < SYN_CLI_HISTORY_DEPTH) {
        cli->history_count++;
    }
    cli->history_read = 0U;
}
#endif

/**
 * @brief Advance ANSI escape state machine; maps ESC [ A to Ctrl-P (0x10).
 * @param cli CLI instance.
 * @param ch  [in,out] Input character (remapped to 0x10 on Up arrow).
 * @return true if character was consumed by escape parser, false otherwise.
 */
static bool cli_filter_escape(SYN_CLI *cli, char *ch)
{
    if (cli->escape_state == 1U) {
        cli->escape_state = (*ch == '[') ? 2U : 0U;
        return true;
    }
    if (cli->escape_state == 2U) {
        cli->escape_state = 0U;
        if (*ch == 'A') {
            *ch = 0x10; /* Map Up arrow to Ctrl-P */
            return false;
        }
        return true; /* Ignore other escape sequences */
    }
    if (*ch == 0x1B) {
        cli->escape_state = 1U;
        return true;
    }
    return false;
}

/**
 * @brief Erase all characters currently in the active input buffer.
 * @param cli CLI instance.
 */
static void cli_erase_line(SYN_CLI *cli)
{
    while (cli->line_pos > 0U) {
        cli->line_pos--;
        if (cli->echo) {
            cli_puts(cli, "\b \b");
        }
    }
}

/**
 * @brief Handle Backspace / DEL on the active input buffer.
 * @param cli CLI instance.
 */
static void cli_backspace(SYN_CLI *cli)
{
    if (cli->line_pos > 0U) {
        cli->line_pos--;
        if (cli->echo) {
            cli_puts(cli, "\b \b");
        }
    }
}

#if SYN_CLI_HISTORY_DEPTH > 0
/**
 * @brief Recall the previous history entry into the active input buffer.
 * @param cli CLI instance.
 */
static void cli_recall_history(SYN_CLI *cli)
{
    if (cli->history_count == 0U) {
        return;
    }
    cli_erase_line(cli);
    if (cli->history_read < cli->history_count) {
        cli->history_read++;
    }
    size_t idx =
        (cli->history_write + SYN_CLI_HISTORY_DEPTH - cli->history_read) % SYN_CLI_HISTORY_DEPTH;
    strncpy(cli->line_buf, cli->history[idx], SYN_CLI_LINE_BUF_SIZE - 1U);
    cli->line_buf[SYN_CLI_LINE_BUF_SIZE - 1U] = '\0';
    cli->line_pos = strlen(cli->line_buf);
    if (cli->echo) {
        cli_puts(cli, cli->line_buf);
    }
}
#endif

/**
 * @brief Append a printable character (or Tab mapped to space) to the line buffer.
 * @param cli CLI instance.
 * @param ch  Character to append.
 */
static void cli_append_char(SYN_CLI *cli, char ch)
{
    if ((unsigned char)ch < 0x20U && ch != '\t') {
        return;
    }
    if (ch == '\t') {
        ch = ' ';
    }
    if (cli->line_pos < SYN_CLI_LINE_BUF_SIZE - 1U) {
        cli->line_buf[cli->line_pos++] = ch;
        if (cli->echo) {
            cli_putchar(cli, ch);
        }
    }
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void syn_cli_init(SYN_CLI *cli, const SYN_CLI_Command *commands, size_t cmd_count,
                  const char *prompt)
{
    if (cli == NULL) {
        return;
    }

    memset(cli, 0, sizeof(*cli));
    cli->commands = commands;
    cli->command_count = (commands != NULL) ? cmd_count : 0U;
    cli->prompt = (prompt != NULL) ? prompt : "> ";
    cli->echo = true;
    cli->line_pos = 0U;
}

void syn_cli_set_echo(SYN_CLI *cli, bool echo)
{
    if (cli == NULL) {
        return;
    }
    cli->echo = echo;
}

/**
 * @brief Terminate current input line, push to history, dispatch command, and redraw prompt.
 * @param cli CLI instance.
 */
static void cli_execute_line(SYN_CLI *cli)
{
    if (cli->echo) {
        cli_puts(cli, "\r\n");
    }

    cli->line_buf[cli->line_pos] = '\0';

#if SYN_CLI_HISTORY_DEPTH > 0
    cli_history_push(cli, cli->line_buf);
#endif

    cli->prompt_visible = false;
    cli->line_pos = 0U;

    cli_dispatch(cli, cli->line_buf);

    syn_cli_print_prompt(cli);
}

void syn_cli_process_char(SYN_CLI *cli, char ch)
{
    if (cli == NULL) {
        return;
    }

    if (cli->last_was_cr) {
        cli->last_was_cr = false;
        if (ch == '\n') {
            return;
        }
    }

    if (cli_filter_escape(cli, &ch)) {
        return;
    }

    /* Newline — execute */
    if (ch == '\r' || ch == '\n') {
        cli->last_was_cr = (ch == '\r');
        cli_execute_line(cli);
        return;
    }

    /* Backspace / DEL */
    if (ch == '\b' || ch == 0x7F) {
        cli_backspace(cli);
        return;
    }

    /* Ctrl-C — cancel current line */
    if (ch == 0x03) {
        if (cli->echo) {
            cli_puts(cli, "^C\r\n");
        }
        cli->line_pos = 0U;
#if SYN_CLI_HISTORY_DEPTH > 0
        cli->history_read = 0U;
#endif
        syn_cli_print_prompt(cli);
        return;
    }

    /* Ctrl-U — clear line */
    if (ch == 0x15) {
        cli_erase_line(cli);
        return;
    }

#if SYN_CLI_HISTORY_DEPTH > 0
    /* Up arrow / Ctrl-P — previous history */
    if (ch == 0x10) {
        cli_recall_history(cli);
        return;
    }
#endif

    cli_append_char(cli, ch);
}

void syn_cli_process_line(SYN_CLI *cli, const char *line)
{
    if (cli == NULL || line == NULL) {
        return;
    }

    /* Copy into the line buffer so we can tokenize in-place */
    strncpy(cli->line_buf, line, SYN_CLI_LINE_BUF_SIZE - 1U);
    cli->line_buf[SYN_CLI_LINE_BUF_SIZE - 1U] = '\0';
    cli->line_pos = 0U;

#if SYN_CLI_HISTORY_DEPTH > 0
    cli_history_push(cli, cli->line_buf);
#endif

    cli_dispatch(cli, cli->line_buf);
}

void syn_cli_print_prompt(SYN_CLI *cli)
{
    if (cli == NULL) {
        return;
    }
    cli_puts(cli, cli->prompt);
    if (cli->line_pos > 0U) {
        cli->line_buf[cli->line_pos] = '\0';
        cli_puts(cli, cli->line_buf);
    }
    cli->prompt_visible = true;
}

void syn_cli_refresh_prompt(SYN_CLI *cli)
{
    syn_cli_print_prompt(cli);
}

#if !defined(SYN_CLI_USE_PRINTF) || SYN_CLI_USE_PRINTF
void syn_cli_printf(SYN_CLI *cli, const char *fmt, ...)
{
    if (cli == NULL || fmt == NULL) {
        return;
    }

    bool was_prompt_visible = cli->prompt_visible;
    if (was_prompt_visible) {
        /* Clear prompt & active draft line once before outputting text */
        cli_puts(cli, "\r\033[K");
        cli->prompt_visible = false;
    }

    char buf[SYN_CLI_LINE_BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n > 0) {
        buf[sizeof(buf) - 1U] = '\0';
        cli_puts(cli, buf);
    }

    if (was_prompt_visible) {
        /* Redraw prompt & draft text after outputting text */
        syn_cli_print_prompt(cli);
    }
}
#endif

/* ── Built-in command implementations ───────────────────────────────────── */

#include "../port/syn_port_system.h"
#include "../system/syn_version.h"
#if SYN_CLI_CMD_ERRORS
#include "../system/syn_errlog.h"
#endif
#if SYN_CLI_CMD_TASKS
#include "../sched/syn_sched.h"
#endif

/* Singletons — defined near top of file, setters here */

#if SYN_CLI_CMD_ERRORS
/** @brief Set errlog instance for the `errors` built-in command. */
void syn_cli_set_errlog(SYN_ErrLog *errlog)
{
    s_cli_errlog = errlog;
}
#endif

#if SYN_CLI_CMD_TASKS
/** @brief Set scheduler instance for the `tasks` built-in command. */
void syn_cli_set_scheduler(SYN_Sched *sched)
{
    s_cli_sched = sched;
}
#endif

#if SYN_CLI_CMD_VERSION
/**
 * @brief Built-in 'version' command handler.
 * @param cli  CLI instance.
 */
static void cli_builtin_version(const SYN_CLI *cli)
{
    char num[12];
    const SYN_Version *v = syn_version();

    cli_puts(cli, v->app_name);
    cli_puts(cli, " v");
    syn_fmt_uint(num, sizeof(num), v->year);
    cli_puts(cli, num);
    cli_puts(cli, ".");
    syn_fmt_uint(num, sizeof(num), v->month);
    cli_puts(cli, num);
    cli_puts(cli, ".");
    syn_fmt_uint(num, sizeof(num), v->release);
    cli_puts(cli, num);
    cli_puts(cli, " (");
    cli_puts(cli, v->date);
    cli_puts(cli, " ");
    cli_puts(cli, v->time);
    cli_puts(cli, ") git:");
    cli_puts(cli, v->git_hash);
    cli_puts(cli, "\r\n");
}
#endif

#if SYN_CLI_CMD_UPTIME
/**
 * @brief Built-in 'uptime' command handler.
 * @param cli  CLI instance.
 */
static void cli_builtin_uptime(const SYN_CLI *cli)
{
    char num[12];
    uint32_t ms = syn_port_get_tick_ms();
    uint32_t secs = ms / 1000;
    uint32_t mins = secs / 60;
    uint32_t hours = mins / 60;
    uint32_t days = hours / 24;

    cli_puts(cli, "Uptime: ");
    syn_fmt_uint(num, sizeof(num), days);
    cli_puts(cli, num);
    cli_puts(cli, "d ");
    syn_fmt_uint(num, sizeof(num), hours % 24);
    cli_puts(cli, num);
    cli_puts(cli, "h ");
    syn_fmt_uint(num, sizeof(num), mins % 60);
    cli_puts(cli, num);
    cli_puts(cli, "m ");
    syn_fmt_uint(num, sizeof(num), secs % 60);
    cli_puts(cli, num);
    cli_puts(cli, "s (");
    syn_fmt_uint(num, sizeof(num), ms);
    cli_puts(cli, num);
    cli_puts(cli, "ms)\r\n");
}
#endif

#if SYN_CLI_CMD_ERRORS
/**
 * @brief Built-in 'errors' command handler.
 * @param cli  CLI instance.
 */
static void cli_builtin_errors(const SYN_CLI *cli)
{
    if (s_cli_errlog == NULL) {
        cli_puts(cli, "No error log configured. Call syn_cli_set_errlog().\r\n");
        return;
    }

    char num[12];
    size_t avail = syn_errlog_available(s_cli_errlog);

    cli_puts(cli, "Error log: ");
    syn_fmt_uint(num, sizeof(num), (uint32_t)syn_errlog_count(s_cli_errlog));
    cli_puts(cli, num);
    cli_puts(cli, " total, ");
    syn_fmt_uint(num, sizeof(num), (uint32_t)avail);
    cli_puts(cli, num);
    cli_puts(cli, " stored\r\n");

    if (avail == 0)
        return;

    static const char *const sev_names[] = {"INFO", "WARN", "ERR ", "FATL"};

    SYN_ErrEntry e;
    for (size_t i = 0; i < avail; i++) {
        if (!syn_errlog_read(s_cli_errlog, i, &e))
            continue; /* LCOV_EXCL_LINE: Unreachable guard; loop bound avail is returned by
                         syn_errlog_count */

        cli_puts(cli, "  [");
        syn_fmt_uint(num, sizeof(num), e.timestamp);
        cli_puts(cli, num);
        cli_puts(cli, "] ");

        const char *sn = (e.severity <= SYN_ERR_FATAL) ? sev_names[e.severity] : "????";
        cli_puts(cli, sn);

        cli_puts(cli, " code=0x");
        syn_fmt_hex(num, sizeof(num), e.code, 4);
        cli_puts(cli, num);

        cli_puts(cli, " ctx=");
        syn_fmt_uint(num, sizeof(num), e.context);
        cli_puts(cli, num);

        cli_puts(cli, " boot=");
        syn_fmt_uint(num, sizeof(num), e.boot_count);
        cli_puts(cli, num);

        cli_puts(cli, "\r\n");
    }
}
#endif

#if SYN_CLI_CMD_TASKS
/**
 * @brief Built-in 'tasks' command handler.
 * @param cli  CLI instance.
 */
static void cli_builtin_tasks(const SYN_CLI *cli)
{
    if (s_cli_sched == NULL) {
        cli_puts(cli, "No scheduler configured. Call syn_cli_set_scheduler().\r\n");
        return;
    }

    char num[12];
    cli_puts(cli, "Tasks:\r\n");

    for (size_t i = 0; i < s_cli_sched->task_count; i++) {
        const SYN_Task *t = &s_cli_sched->tasks[i];
        cli_puts(cli, "  ");
        cli_puts(cli, t->name ? t->name : "(unnamed)");
        cli_puts(cli, "  prio=");
        syn_fmt_uint(num, sizeof(num), t->priority);
        cli_puts(cli, num);

        static const char *const state_names[] = {"  READY", "  SUSPENDED", "  DEAD", "  BLOCKED"};
        const char *sn = (t->state <= (uint8_t)SYN_TASK_BLOCKED) ? state_names[t->state] : "  ???";
        cli_puts(cli, sn);
        cli_puts(cli, "\r\n");
    }
}
#endif

#endif /* SYN_USE_CLI */
