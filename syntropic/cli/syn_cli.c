#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CLI) || SYN_USE_CLI

/**
 * @file syn_cli.c
 * @brief Command-line interpreter implementation.
 */

#include "syn_cli.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"

#include <string.h>

#if !defined(SYN_CLI_USE_PRINTF) || SYN_CLI_USE_PRINTF
#include <stdio.h>
#include <stdarg.h>
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

/**
 * @brief Emit a single character via the CLI output.
 * @param cli  CLI instance.
 * @param ch   Character to emit.
 */
static void cli_putchar(const SYN_CLI *cli, char ch)
{
    if (cli->putchar_fn != NULL) {
        cli->putchar_fn(ch);
    }
}

/**
 * @brief Emit a null-terminated string via the CLI output.
 * @param cli  CLI instance.
 * @param str  String to print.
 */
static void cli_puts(const SYN_CLI *cli, const char *str)
{
    if (str == NULL) return;

    if (cli->puts_fn != NULL) {
        cli->puts_fn(str);
        return;
    }

    /* Fallback: use putchar one char at a time */
    if (cli->putchar_fn != NULL) {
        while (*str) {
            cli->putchar_fn(*str++);
        }
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
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Handle quoted string */
        if (*p == '"') {
            p++; /* skip opening quote */
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) *p++ = '\0';
        }
    }

    return argc;
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
        cli_puts(cli, "Available commands:\r\n");

        /* Built-in help entry */
        cli_puts(cli, "  help           — Show this help\r\n");
#if SYN_CLI_CMD_VERSION
        cli_puts(cli, "  version        — Show build info\r\n");
#endif
#if SYN_CLI_CMD_UPTIME
        cli_puts(cli, "  uptime         — Show system uptime\r\n");
#endif
#if SYN_CLI_CMD_ERRORS
        cli_puts(cli, "  errors         — Dump error log\r\n");
#endif
#if SYN_CLI_CMD_TASKS
        cli_puts(cli, "  tasks          — Show scheduler tasks\r\n");
#endif

        for (size_t i = 0; i < cli->command_count; i++) {
            cli_puts(cli, "  ");
            cli_puts(cli, cli->commands[i].name);
            if (cli->commands[i].help != NULL) {
                /* Pad to align with built-in entries (15 chars for name) */
                size_t name_len = strlen(cli->commands[i].name);
                for (size_t p = name_len; p < 15; p++) {
                    cli_putchar(cli, ' ');
                }
                cli_puts(cli, "— ");
                cli_puts(cli, cli->commands[i].help);
            }
            cli_puts(cli, "\r\n");
        }
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

    /* Search command table */
    for (size_t i = 0; i < cli->command_count; i++) {
        if (strcmp(argv[0], cli->commands[i].name) == 0) {
            if (cli->commands[i].handler != NULL) {
                int ret = cli->commands[i].handler(argc, argv);
                if (ret != 0) {
                    char buf[32];
                    const char *parts[] = { "Error: ", NULL, "\r\n" };
                    char num[12];
                    syn_fmt_int(num, sizeof(num), ret);
                    parts[1] = num;
                    syn_fmt_concat(buf, sizeof(buf), parts, 3);
                    cli_puts(cli, buf);
                }
            }
            return;
        }
    }

    /* Command not found */
    cli_puts(cli, "Unknown command: ");
    cli_puts(cli, argv[0]);
    cli_puts(cli, "\r\nType 'help' for a list of commands.\r\n");
}

#if SYN_CLI_HISTORY_DEPTH > 0
static void cli_history_push(SYN_CLI *cli, const char *line)
{
    if (line[0] == '\0') return;

    /* Don't store duplicates of the last entry */
    if (cli->history_count > 0) {
        size_t last = (cli->history_write + SYN_CLI_HISTORY_DEPTH - 1)
                      % SYN_CLI_HISTORY_DEPTH;
        if (strcmp(cli->history[last], line) == 0) return;
    }

    strncpy(cli->history[cli->history_write], line,
            SYN_CLI_LINE_BUF_SIZE - 1);
    cli->history[cli->history_write][SYN_CLI_LINE_BUF_SIZE - 1] = '\0';

    cli->history_write = (cli->history_write + 1) % SYN_CLI_HISTORY_DEPTH;
    if (cli->history_count < SYN_CLI_HISTORY_DEPTH) {
        cli->history_count++;
    }
    cli->history_read = cli->history_write;
}
#endif

/* ── Public API ─────────────────────────────────────────────────────────── */

void syn_cli_init(SYN_CLI *cli,
                   const SYN_CLI_Command *commands,
                   size_t cmd_count,
                   SYN_CLI_PutChar putchar_fn,
                   const char *prompt)
{
    SYN_ASSERT(cli != NULL);
    SYN_ASSERT(putchar_fn != NULL);

    memset(cli, 0, sizeof(*cli));
    cli->commands      = commands;
    cli->command_count = cmd_count;
    cli->putchar_fn    = putchar_fn;
    cli->puts_fn       = NULL;
    cli->prompt        = (prompt != NULL) ? prompt : "> ";
    cli->echo          = true;
    cli->line_pos      = 0;
}

void syn_cli_set_puts(SYN_CLI *cli, SYN_CLI_Puts puts_fn)
{
    SYN_ASSERT(cli != NULL);
    cli->puts_fn = puts_fn;
}

void syn_cli_set_echo(SYN_CLI *cli, bool echo)
{
    SYN_ASSERT(cli != NULL);
    cli->echo = echo;
}

void syn_cli_process_char(SYN_CLI *cli, char ch)
{
    SYN_ASSERT(cli != NULL);

    /* ANSI escape sequence parsing (e.g. Up Arrow = \x1B[A) */
    if (cli->escape_state == 1) {
        if (ch == '[') cli->escape_state = 2;
        else cli->escape_state = 0;
        return;
    }
    if (cli->escape_state == 2) {
        cli->escape_state = 0;
        if (ch == 'A') ch = 0x10; /* Map Up arrow to Ctrl-P */
        else return;              /* Ignore other arrows for now */
    }
    if (ch == 0x1B) {
        cli->escape_state = 1;
        return;
    }

    /* Newline — execute */
    if (ch == '\r' || ch == '\n') {
        if (cli->echo) {
            cli_puts(cli, "\r\n");
        }

        cli->line_buf[cli->line_pos] = '\0';

#if SYN_CLI_HISTORY_DEPTH > 0
        cli_history_push(cli, cli->line_buf);
#endif

        cli_dispatch(cli, cli->line_buf);

        cli->line_pos = 0;
        syn_cli_print_prompt(cli);
        return;
    }

    /* Backspace / DEL */
    if (ch == '\b' || ch == 0x7F) {
        if (cli->line_pos > 0) {
            cli->line_pos--;
            if (cli->echo) {
                cli_puts(cli, "\b \b"); /* erase character on terminal */
            }
        }
        return;
    }

    /* Ctrl-C — cancel current line */
    if (ch == 0x03) {
        if (cli->echo) {
            cli_puts(cli, "^C\r\n");
        }
        cli->line_pos = 0;
        syn_cli_print_prompt(cli);
        return;
    }

    /* Ctrl-U — clear line */
    if (ch == 0x15) {
        while (cli->line_pos > 0) {
            cli->line_pos--;
            if (cli->echo) {
                cli_puts(cli, "\b \b");
            }
        }
        return;
    }

#if SYN_CLI_HISTORY_DEPTH > 0
    /* Up arrow (simplified: Ctrl-P) — previous history */
    if (ch == 0x10) { /* Ctrl-P */
        if (cli->history_count > 0) {
            /* Erase current line */
            while (cli->line_pos > 0) {
                cli->line_pos--;
                if (cli->echo) cli_puts(cli, "\b \b");
            }
            /* Navigate backwards */
            cli->history_read = (cli->history_read + SYN_CLI_HISTORY_DEPTH - 1)
                                % SYN_CLI_HISTORY_DEPTH;
            if (cli->history_read >= cli->history_count) {
                cli->history_read = cli->history_count - 1;
            }
            strncpy(cli->line_buf, cli->history[cli->history_read],
                    SYN_CLI_LINE_BUF_SIZE - 1);
            cli->line_pos = strlen(cli->line_buf);
            if (cli->echo) cli_puts(cli, cli->line_buf);
        }
        return;
    }
#endif

    /* Ignore other control characters */
    if (ch < 0x20 && ch != '\t') {
        return;
    }

    /* Tab — treat as space */
    if (ch == '\t') {
        ch = ' ';
    }

    /* Normal character — append to buffer */
    if (cli->line_pos < SYN_CLI_LINE_BUF_SIZE - 1) {
        cli->line_buf[cli->line_pos++] = ch;
        if (cli->echo) {
            cli_putchar(cli, ch);
        }
    }
    /* Silently drop if buffer is full */
}

void syn_cli_process_line(SYN_CLI *cli, const char *line)
{
    SYN_ASSERT(cli != NULL);
    SYN_ASSERT(line != NULL);

    /* Copy into the line buffer so we can tokenize in-place */
    strncpy(cli->line_buf, line, SYN_CLI_LINE_BUF_SIZE - 1);
    cli->line_buf[SYN_CLI_LINE_BUF_SIZE - 1] = '\0';
    cli->line_pos = 0;

    cli_dispatch(cli, cli->line_buf);
}

void syn_cli_print_prompt(const SYN_CLI *cli)
{
    SYN_ASSERT(cli != NULL);
    cli_puts(cli, cli->prompt);
}

#if !defined(SYN_CLI_USE_PRINTF) || SYN_CLI_USE_PRINTF
void syn_cli_printf(const SYN_CLI *cli, const char *fmt, ...)
{
    SYN_ASSERT(cli != NULL);

    char buf[SYN_CLI_LINE_BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n > 0) {
        buf[sizeof(buf) - 1] = '\0';
        cli_puts(cli, buf);
    }
}
#endif

/* ── Built-in command implementations ───────────────────────────────────── */

#include "../system/syn_version.h"
#include "../port/syn_port_system.h"
#include "../system/syn_errlog.h"
#include "../sched/syn_sched.h"

/* Singletons (set via syn_cli_set_*) */
static SYN_ErrLog *s_cli_errlog = NULL;  /**< Error log instance for 'errors' command. */
static SYN_Sched  *s_cli_sched  = NULL;  /**< Scheduler for 'tasks' command. */

void syn_cli_set_errlog(SYN_ErrLog *errlog)  { s_cli_errlog = errlog; }
void syn_cli_set_scheduler(SYN_Sched *sched) { s_cli_sched = sched; }

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
    syn_fmt_uint(num, sizeof(num), v->major);  cli_puts(cli, num);
    cli_puts(cli, ".");
    syn_fmt_uint(num, sizeof(num), v->minor);  cli_puts(cli, num);
    cli_puts(cli, ".");
    syn_fmt_uint(num, sizeof(num), v->patch);  cli_puts(cli, num);
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
    uint32_t secs  = ms / 1000;
    uint32_t mins  = secs / 60;
    uint32_t hours = mins / 60;
    uint32_t days  = hours / 24;

    cli_puts(cli, "Uptime: ");
    syn_fmt_uint(num, sizeof(num), days);   cli_puts(cli, num); cli_puts(cli, "d ");
    syn_fmt_uint(num, sizeof(num), hours % 24); cli_puts(cli, num); cli_puts(cli, "h ");
    syn_fmt_uint(num, sizeof(num), mins % 60);  cli_puts(cli, num); cli_puts(cli, "m ");
    syn_fmt_uint(num, sizeof(num), secs % 60);  cli_puts(cli, num); cli_puts(cli, "s (");
    syn_fmt_uint(num, sizeof(num), ms);     cli_puts(cli, num); cli_puts(cli, "ms)\r\n");
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

    if (avail == 0) return;

    static const char * const sev_names[] = { "INFO", "WARN", "ERR ", "FATL" };

    SYN_ErrEntry e;
    for (size_t i = 0; i < avail; i++) {
        if (!syn_errlog_read(s_cli_errlog, i, &e)) continue;

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

    for (uint8_t i = 0; i < s_cli_sched->task_count; i++) {
        const SYN_Task *t = &s_cli_sched->tasks[i];
        cli_puts(cli, "  ");
        cli_puts(cli, t->name ? t->name : "(unnamed)");
        cli_puts(cli, "  prio=");
        syn_fmt_uint(num, sizeof(num), t->priority);
        cli_puts(cli, num);

        static const char * const state_names[] = {
            "  READY", "  SUSPENDED", "  DEAD"
        };
        const char *sn = (t->state <= 2) ? state_names[t->state] : "  ???";
        cli_puts(cli, sn);
        cli_puts(cli, "\r\n");
    }
}
#endif

#endif /* SYN_USE_CLI */
