/**
 * @file syn_log.h
 * @brief Severity-filtered logging system.
 *
 * Provides printf-style logging with severity levels, per-module tags,
 * optional timestamps, and ANSI color output. Logging calls below the
 * compile-time minimum level are stripped entirely (zero overhead).
 *
 * @par Output backend
 * The user provides an output function at init time (typically wrapping
 * UART transmit). If no init is called, a weak default discards output.
 *
 * @par Compile-time filtering
 * Set `SYN_LOG_LEVEL` in syn_config.h to the minimum level. Any
 * macro call below that level compiles to `((void)0)`.
 *
 * @par Usage
 * @code
 *   static void my_output(const char *str, size_t len) {
 *       syn_port_uart_transmit(0, (const uint8_t *)str, len, 0);
 *   }
 *
 *   syn_log_init(my_output, SYN_LOG_DEBUG);
 *
 *   #define TAG "main"
 *   SYN_LOG_I(TAG, "System started, version %d.%d", 1, 0);
 *   SYN_LOG_D(TAG, "Free memory: %u bytes", free_mem);
 *   SYN_LOG_E(TAG, "Sensor read failed: %d", status);
 * @endcode
 *
 * Output:
 * @code
 *   [   1234] I/main: System started, version 1.0
 *   [   1235] D/main: Free memory: 8192 bytes
 *   [   1240] E/main: Sensor read failed: -1
 * @endcode
 * @ingroup syn_debug
 */

#ifndef SYN_LOG_H
#define SYN_LOG_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Log levels ─────────────────────────────────────────────────────────── */

/** @brief Log severity levels. */
typedef enum {
    SYN_LOG_TRACE = 0,   /**< Very fine-grained debug info          */
    SYN_LOG_DEBUG = 1,   /**< Debug messages                         */
    SYN_LOG_INFO  = 2,   /**< Informational messages                 */
    SYN_LOG_WARN  = 3,   /**< Warnings                               */
    SYN_LOG_ERROR = 4,   /**< Errors                                  */
    SYN_LOG_FATAL = 5,   /**< Fatal / unrecoverable errors           */
    SYN_LOG_NONE  = 6,   /**< Disable all logging                    */
} SYN_LogLevel;

/* ── Compile-time minimum level ─────────────────────────────────────────── */

#ifndef SYN_LOG_LEVEL
  /** @brief Compile-time minimum log level (defaults to SYN_LOG_DEBUG). */
  #define SYN_LOG_LEVEL   SYN_LOG_DEBUG
#endif

/* ── Output buffer size ─────────────────────────────────────────────────── */

#ifndef SYN_LOG_BUF_SIZE
  /** @brief Size of the log formatting buffer in bytes. */
  #define SYN_LOG_BUF_SIZE   192
#endif

/* ── Timestamp ──────────────────────────────────────────────────────────── */

/** Set to 0 in syn_config.h to disable timestamp prefix. */
#ifndef SYN_LOG_TIMESTAMP
  #define SYN_LOG_TIMESTAMP  1
#endif

/* ── Color output ───────────────────────────────────────────────────────── */

/** Set to 1 in syn_config.h to enable ANSI color codes. */
#ifndef SYN_LOG_COLOR
  #define SYN_LOG_COLOR      0
#endif

/* ── Output function type ───────────────────────────────────────────────── */

/**
 * @brief Log output callback.
 *
 * The logging system calls this to emit formatted output. The string
 * is NOT null-terminated at position @p len (it is within the buffer,
 * but @p len is the exact number of chars to write).
 *
 * @param str  Formatted string.
 * @param len  Number of bytes to output.
 */
typedef void (*SYN_LogOutputFunc)(const char *str, size_t len);

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the logging system.
 *
 * @param output    Function that writes log output (e.g., to UART).
 * @param min_level Minimum runtime log level (messages below this are
 *                  suppressed at runtime). Compile-time level still applies.
 */
void syn_log_init(SYN_LogOutputFunc output, SYN_LogLevel min_level);

/**
 * @brief Change the runtime minimum log level.
 * @param level  New minimum level.
 */
void syn_log_set_level(SYN_LogLevel level);

/**
 * @brief Get the current runtime minimum log level.
 * @return Current minimum level.
 */
SYN_LogLevel syn_log_get_level(void);

/**
 * @brief Core log function (printf-style).
 *
 * Normally called via the SYN_LOG_* macros rather than directly.
 *
 * @param level  Severity level.
 * @param tag    Module/component name (short string).
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void syn_log(SYN_LogLevel level, const char *tag, const char *fmt, ...);

/**
 * @brief Core log function (va_list variant).
 * @param level  Severity level.
 * @param tag    Module/component name.
 * @param fmt    printf-style format string.
 * @param args   va_list of format arguments.
 */
void syn_log_va(SYN_LogLevel level, const char *tag, const char *fmt, va_list args);

/**
 * @brief Output a raw string with no formatting or prefix.
 *
 * Useful for printing hex dumps, tables, or other pre-formatted data
 * that shouldn't get a timestamp/level prefix.
 *
 * @param str  Null-terminated string to output.
 */
void syn_log_raw(const char *str);

/**
 * @brief Hex dump utility — output a buffer as hex + ASCII.
 *
 * @param tag    Log tag.
 * @param data   Buffer to dump.
 * @param len    Number of bytes.
 */
void syn_log_hexdump(const char *tag, const void *data, size_t len);

/** @name Convenience logging macros
 * Calls below the compile-time SYN_LOG_LEVEL are compiled out entirely.
 * @{
 */
#if SYN_LOG_LEVEL <= 0
  /** @brief Log at TRACE level. */
  #define SYN_LOG_T(tag, fmt, ...)   syn_log(SYN_LOG_TRACE, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at TRACE level (compiled out). */
  #define SYN_LOG_T(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 1
  /** @brief Log at DEBUG level. */
  #define SYN_LOG_D(tag, fmt, ...)   syn_log(SYN_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at DEBUG level (compiled out). */
  #define SYN_LOG_D(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 2
  /** @brief Log at INFO level. */
  #define SYN_LOG_I(tag, fmt, ...)   syn_log(SYN_LOG_INFO, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at INFO level (compiled out). */
  #define SYN_LOG_I(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 3
  /** @brief Log at WARN level. */
  #define SYN_LOG_W(tag, fmt, ...)   syn_log(SYN_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at WARN level (compiled out). */
  #define SYN_LOG_W(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 4
  /** @brief Log at ERROR level. */
  #define SYN_LOG_E(tag, fmt, ...)   syn_log(SYN_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at ERROR level (compiled out). */
  #define SYN_LOG_E(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 5
  /** @brief Log at FATAL level. */
  #define SYN_LOG_F(tag, fmt, ...)   syn_log(SYN_LOG_FATAL, tag, fmt, ##__VA_ARGS__)
#else
  /** @brief Log at FATAL level (compiled out). */
  #define SYN_LOG_F(tag, fmt, ...)   ((void)0)
#endif
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SYN_LOG_H */
