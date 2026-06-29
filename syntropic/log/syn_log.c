#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_LOG) || SYN_USE_LOG

/**
 * @file syn_log.c
 * @brief Logging system implementation.
 */

#include "syn_log.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"

#if defined(SYN_USE_FMT) && !SYN_USE_FMT
  #error "syn_log requires SYN_USE_FMT=1. Enable it in syn_config.h."
#endif

#if !defined(SYN_LOG_USE_PRINTF) || SYN_LOG_USE_PRINTF
#include <stdio.h>
#endif
#include <string.h>

/* ── State ──────────────────────────────────────────────────────────────── */

static SYN_LogOutputFunc s_output   = NULL;        /**< Registered output callback. */
static SYN_LogLevel      s_level    = SYN_LOG_DEBUG; /**< Current minimum log level.  */

/* ── Level labels ───────────────────────────────────────────────────────── */

/** @brief Single-char level labels: T=Trace, D=Debug, I=Info, W=Warn, E=Error, F=Fatal. */
static const char * const s_level_chars = "TDIWEF";

#if SYN_LOG_COLOR
static const char * const s_level_colors[] = {
    "\033[90m",     /* TRACE — gray     */
    "\033[36m",     /* DEBUG — cyan     */
    "\033[32m",     /* INFO  — green    */
    "\033[33m",     /* WARN  — yellow   */
    "\033[31m",     /* ERROR — red      */
    "\033[1;31m",   /* FATAL — bold red */
};
#define LOG_COLOR_RESET   "\033[0m"
#endif

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_log_init(SYN_LogOutputFunc output, SYN_LogLevel min_level)
{
    s_output = output;
    s_level  = min_level;
}

void syn_log_set_level(SYN_LogLevel level)
{
    s_level = level;
}

SYN_LogLevel syn_log_get_level(void)
{
    return s_level;
}

void syn_log_va(SYN_LogLevel level, const char *tag, const char *fmt, va_list args)
{
    if (s_output == NULL) {
        return;
    }

    if (level < s_level) {
        return;
    }

    char buf[SYN_LOG_BUF_SIZE];
    int  pos = 0;
    int  remaining = (int)sizeof(buf) - 2; /* reserve space for \n\0 */

    /* Color prefix */
#if SYN_LOG_COLOR
    if (level <= SYN_LOG_FATAL) {
        int n = snprintf(buf + pos, (size_t)(remaining - pos),
                         "%s", s_level_colors[level]);
        if (n > 0) pos += n;
    }
#endif

    /* Timestamp */
#if SYN_LOG_TIMESTAMP
    {
        uint32_t tick = syn_port_get_tick_ms();
        char num[12];
        buf[pos++] = '[';
        syn_fmt_uint(num, sizeof(num), tick);
        size_t nl = strlen(num);
        size_t pad = (7 > nl) ? 7 - nl : 0;
        while (pad-- > 0) buf[pos++] = ' ';
        memcpy(buf + pos, num, nl); pos += (int)nl;
        buf[pos++] = ']';
        buf[pos++] = ' ';
    }
#endif

    /* Level + tag */
    {
        char lvl = (level <= SYN_LOG_FATAL) ? s_level_chars[level] : '?';
        buf[pos++] = lvl;
        if (tag != NULL && tag[0] != '\0') {
            buf[pos++] = '/';
            size_t tl = strlen(tag);
            if (tl > (size_t)(remaining - pos - 3)) tl = (size_t)(remaining - pos - 3);
            memcpy(buf + pos, tag, tl); pos += (int)tl;
        }
        buf[pos++] = ':';
        buf[pos++] = ' ';
    }

    /* User message */
    {
#if !defined(SYN_LOG_USE_PRINTF) || SYN_LOG_USE_PRINTF
        int n = vsnprintf(buf + pos, (size_t)(remaining - pos), fmt, args);
        if (n > 0) {
            pos += n;
            if (pos > remaining) pos = remaining;
        }
#else
        /* No printf: copy format string literally */
        size_t fl = strlen(fmt);
        if (fl > (size_t)(remaining - pos)) fl = (size_t)(remaining - pos);
        memcpy(buf + pos, fmt, fl);
        pos += (int)fl;
#endif
    }

    /* Color reset */
#if SYN_LOG_COLOR
    {
        const char *rst = LOG_COLOR_RESET "\n";
        size_t rl = strlen(rst);
        if (rl > sizeof(buf) - (size_t)pos - 1) rl = sizeof(buf) - (size_t)pos - 1;
        memcpy(buf + pos, rst, rl);
        pos += (int)rl;
    }
#else
    /* Newline */
    buf[pos++] = '\n';
#endif

    /* Null-terminate */
    if (pos >= (int)sizeof(buf)) pos = (int)sizeof(buf) - 1;
    buf[pos] = '\0';

    s_output(buf, (size_t)pos);
}

void syn_log(SYN_LogLevel level, const char *tag, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    syn_log_va(level, tag, fmt, args);
    va_end(args);
}

void syn_log_raw(const char *str)
{
    if (s_output == NULL || str == NULL) {
        return;
    }
    s_output(str, strlen(str));
}

void syn_log_hexdump(const char *tag, const void *data, size_t len)
{
    if (s_output == NULL || data == NULL) {
        return;
    }

    const uint8_t *bytes = (const uint8_t *)data;
    char line[80];
    size_t offset = 0;

    while (offset < len) {
        size_t pos = 0;
        char num[4];

        /* Offset: XXXX  */
        syn_fmt_hex(num, sizeof(num), (uint32_t)offset, 4);
        memcpy(line + pos, num, 4); pos += 4;
        line[pos++] = ' ';
        line[pos++] = ' ';

        /* Hex bytes */
        size_t i;
        for (i = 0; i < 16; i++) {
            if (offset + i < len) {
                syn_fmt_hex(num, sizeof(num), bytes[offset + i], 2);
                line[pos++] = num[0];
                line[pos++] = num[1];
                line[pos++] = ' ';
            } else {
                line[pos++] = ' ';
                line[pos++] = ' ';
                line[pos++] = ' ';
            }
            if (i == 7) {
                line[pos++] = ' ';
            }
        }

        /* ASCII */
        line[pos++] = '|';
        for (i = 0; i < 16 && (offset + i) < len; i++) {
            uint8_t c = bytes[offset + i];
            line[pos++] = (c >= 0x20 && c <= 0x7E) ? (char)c : '.';
        }
        line[pos++] = '|';
        line[pos++] = '\n';
        line[pos]   = '\0';

        syn_log(SYN_LOG_DEBUG, tag, "%s", line);

        offset += 16;
    }
}

#endif /* SYN_USE_LOG */
