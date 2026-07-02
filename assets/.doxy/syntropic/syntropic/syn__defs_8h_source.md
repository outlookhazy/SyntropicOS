

# File syn\_defs.h

[**File List**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_defs.h**](syn__defs_8h.md)

[Go to the documentation of this file](syn__defs_8h.md)


```C++

#ifndef SYN_DEFS_H
#define SYN_DEFS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Boolean ────────────────────────────────────────────────────────────── */

#ifndef __cplusplus
  #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    #include <stdbool.h>
  #else
    typedef enum { false = 0, true = 1 } bool;
  #endif
#endif

/* ── Status codes ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_OK              = 0,   
    SYN_ERROR           = -1,  
    SYN_BUSY            = -2,  
    SYN_TIMEOUT         = -3,  
    SYN_INVALID_PARAM   = -4,  
    SYN_NOT_IMPLEMENTED = -5,  
} SYN_Status;

/* ── GPIO types ─────────────────────────────────────────────────────────── */

typedef uint16_t SYN_GPIO_Pin;

typedef enum {
    SYN_GPIO_LOW  = 0,
    SYN_GPIO_HIGH = 1,
} SYN_GPIO_State;

typedef enum {
    SYN_GPIO_INPUT          = 0,
    SYN_GPIO_OUTPUT         = 1,
    SYN_GPIO_INPUT_PULLUP   = 2,
    SYN_GPIO_INPUT_PULLDOWN = 3,
    SYN_GPIO_OUTPUT_OD      = 4,  
} SYN_GPIO_Mode;

/* ── UART types ─────────────────────────────────────────────────────────── */

typedef uint8_t SYN_UARTInstance;

/* ── Utility macros ─────────────────────────────────────────────────────── */

#define SYN_ARRAY_SIZE(arr)   (sizeof(arr) / sizeof((arr)[0]))

#define SYN_UNUSED_VAR(x)     ((void)(x))

#define SYN_MIN(a, b)         (((a) < (b)) ? (a) : (b))
#define SYN_MAX(a, b)         (((a) > (b)) ? (a) : (b))

#define SYN_ABS(x)            (((x) < 0) ? -(x) : (x))

#define SYN_SIGN(x)           (((x) > 0) - ((x) < 0))

#define SYN_CLAMP(val, lo, hi) (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))

#ifdef __cplusplus
}
#endif

#endif /* SYN_DEFS_H */
```


