

# File syn\_lut.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lut.h**](syn__lut_8h.md)

[Go to the documentation of this file](syn__lut_8h.md)


```C++

#ifndef SYN_LUT_H
#define SYN_LUT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Table entry ────────────────────────────────────────────────────────── */

typedef struct {
    int32_t x;  
    int32_t y;  
} SYN_LUT_Entry;

/* ── Forward lookup: x → y (interpolated) ───────────────────────────────── */

static inline int32_t syn_lut_forward(const SYN_LUT_Entry *table,
                                       size_t count, int32_t x)
{
    if (count == 0) return 0;
    if (count == 1 || x <= table[0].x) return table[0].y;
    if (x >= table[count - 1].x)       return table[count - 1].y;

    /* Find the bracketing interval */
    size_t i;
    for (i = 1; i < count; i++) {
        if (x <= table[i].x) break;
    }

    /* Linear interpolation */
    int32_t x0 = table[i - 1].x, y0 = table[i - 1].y;
    int32_t x1 = table[i].x,     y1 = table[i].y;
    int32_t dx = x1 - x0;

    if (dx == 0) return y0;

    return y0 + ((y1 - y0) * (x - x0)) / dx;
}

/* ── Reverse lookup: y → x (interpolated) ───────────────────────────────── */

static inline int32_t syn_lut_reverse(const SYN_LUT_Entry *table,
                                       size_t count, int32_t y)
{
    if (count == 0) return 0;
    if (count == 1) return table[0].x;

    /* Detect direction (is y ascending or descending?) */
    int ascending = (table[count - 1].y >= table[0].y) ? 1 : 0;

    /* Find the bracketing interval */
    size_t i;
    for (i = 1; i < count; i++) {
        if (ascending) {
            if (y <= table[i].y) break;
        } else {
            if (y >= table[i].y) break;
        }
    }

    if (i >= count) i = count - 1;

    int32_t x0 = table[i - 1].x, y0 = table[i - 1].y;
    int32_t x1 = table[i].x,     y1 = table[i].y;
    int32_t dy = y1 - y0;

    if (dy == 0) return x0;

    return x0 + ((x1 - x0) * (y - y0)) / dy;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_LUT_H */
```


