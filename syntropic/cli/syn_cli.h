/**
 * @file syn_cli.h
 * @brief Command-line interpreter for embedded systems.
 *
 * Provides a lightweight interactive shell over UART (or any byte stream).
 * Commands are registered as a static table — no dynamic allocation.
 * Input is processed one character at a time, making it easy to feed from
 * a UART ISR or polled loop.
 *
 * @par Features
 * - Statically-registered command table (name + help + handler)
 * - Automatic argc/argv parsing (whitespace-delimited, with quoted strings)
 * - Built-in `help` command
 * - Backspace and line editing
 * - Configurable prompt
 * - Echo control (for terminals that don't echo locally)
 * - Command history (optional, configurable depth)
 *
 * @par Usage
 * @code
 *   // Define commands
 *   static int cmd_led(int argc, char *argv[]);
 *   static int cmd_reset(int argc, char *argv[]);
 *
 *   static const SYN_CLI_Command commands[] = {
 *       { "led",   "led <on|off>  — Control the LED",  cmd_led   },
 *       { "reset", "reset         — System reset",     cmd_reset },
 *   };
 *
 *   // Initialize
 *   static SYN_CLI cli;
 *   syn_cli_init(&cli, commands, 2, my_putchar, "> ");
 *
 *   // In main loop or UART RX ISR:
 *   syn_cli_process_char(&cli, received_byte);
 * @endcode
 * @ingroup syn_debug
 */

#ifndef SYN_CLI_H
#define SYN_CLI_H

#include "../common/syn_defs.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** Maximum length of a single command line (including null terminator). */
#ifndef SYN_CLI_LINE_BUF_SIZE
  #define SYN_CLI_LINE_BUF_SIZE   128
#endif

/** Maximum number of arguments (including the command name). */
#ifndef SYN_CLI_MAX_ARGS
  #define SYN_CLI_MAX_ARGS        16
#endif

/** Command history depth (0 = disabled). */
#ifndef SYN_CLI_HISTORY_DEPTH
  #define SYN_CLI_HISTORY_DEPTH    0
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

/**
 * @brief Command handler function.
 *
 * @param argc  Argument count (including the command name).
 * @param argv  Argument vector. argv[0] is the command name.
 * @return 0 on success, or an application-defined error code.
 */
typedef int (*SYN_CLI_Handler)(int argc, char *argv[]);

/**
 * @brief Command descriptor.
 *
 * Typically defined as a const static array.
 */
typedef struct {
    const char       *name;     /**< Command name (matched against input)   */
    const char       *help;     /**< Help text shown by the `help` command  */
    SYN_CLI_Handler  handler;  /**< Function called when command matches   */
} SYN_CLI_Command;

/**
 * @brief Character output function.
 *
 * The CLI calls this to emit one character (for echo, prompt, and output).
 *
 * @param ch  Character to output.
 */
typedef void (*SYN_CLI_PutChar)(char ch);

/**
 * @brief String output function.
 *
 * The CLI calls this to emit a null-terminated string.
 *
 * @param str  Null-terminated string.
 */
typedef void (*SYN_CLI_Puts)(const char *str);

/** @brief CLI instance — command table, line buffer, I/O, and history. */
typedef struct {
    /* Command table */
    const SYN_CLI_Command *commands;     /**< Registered command array    */
    size_t                  command_count; /**< Number of commands         */

    /* I/O */
    SYN_CLI_PutChar        putchar_fn;   /**< Single-char output function */
    SYN_CLI_Puts           puts_fn;      /**< String output function      */

    /* Line buffer */
    char    line_buf[SYN_CLI_LINE_BUF_SIZE]; /**< Input line buffer       */
    size_t  line_pos;                    /**< Current cursor position     */

    /* Configuration */
    const char *prompt;                  /**< Prompt string               */
    bool        echo;                    /**< Echo enabled                */
    uint8_t     escape_state;            /**< ANSI escape sequence state  */

    /* History */
#if SYN_CLI_HISTORY_DEPTH > 0
    char    history[SYN_CLI_HISTORY_DEPTH][SYN_CLI_LINE_BUF_SIZE]; /**< History ring  */
    size_t  history_count;               /**< Number of history entries   */
    size_t  history_write;               /**< Next write position         */
    size_t  history_read;                /**< Current read position       */
#endif
} SYN_CLI;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize a CLI instance.
 *
 * @param cli        CLI instance to initialize.
 * @param commands   Array of command descriptors.
 * @param cmd_count  Number of commands.
 * @param putchar_fn Character output function.
 * @param prompt     Prompt string (e.g., "> " or "syntropic> "). Stored by
 *                   pointer, not copied.
 */
void syn_cli_init(SYN_CLI *cli,
                   const SYN_CLI_Command *commands,
                   size_t cmd_count,
                   SYN_CLI_PutChar putchar_fn,
                   const char *prompt);

/**
 * @brief Set an optional string output function for multi-char output.
 *
 * If not set, the CLI uses putchar_fn to output strings one character
 * at a time. Setting this can improve efficiency.
 *
 * @param cli      CLI instance.
 * @param puts_fn  String output function.
 */
void syn_cli_set_puts(SYN_CLI *cli, SYN_CLI_Puts puts_fn);

/**
 * @brief Enable or disable local echo.
 *
 * When enabled (default), the CLI echoes each received character back
 * to the terminal. Disable if the terminal handles its own echo.
 *
 * @param cli   CLI instance.
 * @param echo  true to enable echo, false to disable.
 */
void syn_cli_set_echo(SYN_CLI *cli, bool echo);

/**
 * @brief Process a single received character.
 *
 * Call this for each byte received from the input stream (UART RX ISR,
 * polled read, etc.). The CLI handles line editing (backspace) and
 * dispatches the command when a newline is received.
 *
 * @param cli  CLI instance.
 * @param ch   Received character.
 */
void syn_cli_process_char(SYN_CLI *cli, char ch);

/**
 * @brief Process a complete null-terminated line.
 *
 * Alternative to process_char when you already have a full line
 * (e.g., from a buffered read). Does not echo or handle editing.
 *
 * @param cli   CLI instance.
 * @param line  Null-terminated command line.
 */
void syn_cli_process_line(SYN_CLI *cli, const char *line);

/**
 * @brief Print the prompt.
 *
 * Called automatically after command execution. Can also be called
 * manually (e.g., on startup to show the initial prompt).
 *
 * @param cli  CLI instance.
 */
void syn_cli_print_prompt(const SYN_CLI *cli);

/**
 * @brief Print formatted output through the CLI's output function.
 *
 * Convenience function for command handlers that need printf-style output.
 *
 * @param cli  CLI instance.
 * @param fmt  Format string.
 * @param ...  Arguments.
 */
void syn_cli_printf(const SYN_CLI *cli, const char *fmt, ...);

/* ── Built-in commands ──────────────────────────────────────────────────── */

/**
 * @brief Register built-in diagnostic commands.
 *
 * Adds framework-aware commands to the CLI. Each is individually
 * guarded by a define (default: all enabled). The commands operate
 * on the framework singletons you pass in.
 *
 * @param cli       CLI instance.
 *
 * Built-in commands (enable/disable via defines):
 * - `version`  — print syn_version() info     (SYN_CLI_CMD_VERSION, default 1)
 * - `uptime`   — print tick_ms uptime           (SYN_CLI_CMD_UPTIME,  default 1)
 * - `errors`   — dump errlog entries            (SYN_CLI_CMD_ERRORS,  default 1)
 * - `tasks`    — show scheduler task states     (SYN_CLI_CMD_TASKS,   default 1)
 */

#ifndef SYN_CLI_CMD_VERSION
#define SYN_CLI_CMD_VERSION  1
#endif
#ifndef SYN_CLI_CMD_UPTIME
/** @brief Enable built-in 'uptime' command. */
#define SYN_CLI_CMD_UPTIME   1
#endif
#ifndef SYN_CLI_CMD_ERRORS
/** @brief Enable built-in 'errors' command. */
#define SYN_CLI_CMD_ERRORS   1
#endif
#ifndef SYN_CLI_CMD_TASKS
/** @brief Enable built-in 'tasks' command. */
#define SYN_CLI_CMD_TASKS    1
#endif

/**
 * @brief Set errlog instance for the `errors` built-in command.
 *
 * If set, the `errors` command will dump all entries from this log.
 *
 * @param errlog  Error log instance.
 */
#include "../system/syn_errlog.h"
void syn_cli_set_errlog(SYN_ErrLog *errlog);

/**
 * @brief Set scheduler instance for the `tasks` built-in command.
 *
 * If set, the `tasks` command will list all registered tasks.
 *
 * @param sched  Scheduler instance.
 */
#include "../sched/syn_sched.h"
void syn_cli_set_scheduler(SYN_Sched *sched);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CLI_H */
