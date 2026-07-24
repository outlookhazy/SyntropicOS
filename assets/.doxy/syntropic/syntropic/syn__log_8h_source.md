

# File syn\_log.h

[**File List**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_log.h**](syn__log_8h.md)

[Go to the documentation of this file](syn__log_8h.md)


```C++

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

typedef enum {
    SYN_LOG_TRACE = 0,   
    SYN_LOG_DEBUG = 1,   
    SYN_LOG_INFO  = 2,   
    SYN_LOG_WARN  = 3,   
    SYN_LOG_ERROR = 4,   
    SYN_LOG_FATAL = 5,   
    SYN_LOG_NONE  = 6,   
} SYN_LogLevel;

/* ── Compile-time minimum level ─────────────────────────────────────────── */

#ifndef SYN_LOG_LEVEL
  #define SYN_LOG_LEVEL   SYN_LOG_DEBUG
#endif

/* ── Output buffer size ─────────────────────────────────────────────────── */

#ifndef SYN_LOG_BUF_SIZE
  #define SYN_LOG_BUF_SIZE   192
#endif

/* ── Timestamp ──────────────────────────────────────────────────────────── */

#ifndef SYN_LOG_TIMESTAMP
  #define SYN_LOG_TIMESTAMP  1
#endif

/* ── Color output ───────────────────────────────────────────────────────── */

#ifndef SYN_LOG_COLOR
  #define SYN_LOG_COLOR      0
#endif

/* ── Output function type ───────────────────────────────────────────────── */

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_log_init(SYN_LogLevel min_level);

void syn_log_set_level(SYN_LogLevel level);

SYN_LogLevel syn_log_get_level(void);

void syn_log(SYN_LogLevel level, const char *tag, const char *fmt, ...);

void syn_log_va(SYN_LogLevel level, const char *tag, const char *fmt, va_list args);

void syn_log_raw(const char *str);

void syn_log_hexdump(const char *tag, const void *data, size_t len);

#if SYN_LOG_LEVEL <= 0
  #define SYN_LOG_T(tag, fmt, ...)   syn_log(SYN_LOG_TRACE, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_T(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 1
  #define SYN_LOG_D(tag, fmt, ...)   syn_log(SYN_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_D(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 2
  #define SYN_LOG_I(tag, fmt, ...)   syn_log(SYN_LOG_INFO, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_I(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 3
  #define SYN_LOG_W(tag, fmt, ...)   syn_log(SYN_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_W(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 4
  #define SYN_LOG_E(tag, fmt, ...)   syn_log(SYN_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_E(tag, fmt, ...)   ((void)0)
#endif

#if SYN_LOG_LEVEL <= 5
  #define SYN_LOG_F(tag, fmt, ...)   syn_log(SYN_LOG_FATAL, tag, fmt, ##__VA_ARGS__)
#else
  #define SYN_LOG_F(tag, fmt, ...)   ((void)0)
#endif
#ifdef __cplusplus
}
#endif

#endif /* SYN_LOG_H */
```


