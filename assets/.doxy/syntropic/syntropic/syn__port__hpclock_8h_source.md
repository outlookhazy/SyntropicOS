

# File syn\_port\_hpclock.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_hpclock.h**](syn__port__hpclock_8h.md)

[Go to the documentation of this file](syn__port__hpclock_8h.md)


```C++

#ifndef SYN_PORT_HPCLOCK_H
#define SYN_PORT_HPCLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Overflow counter ──────────────────────────────────────────────────── */

extern volatile uint32_t syn_hpclock_msb;

#define SYN_HPCLOCK_OVERFLOW_TICK() (syn_hpclock_msb++)

/* ── Port functions ────────────────────────────────────────────────────── */

volatile uint32_t *syn_port_hpclock_lsb_ptr(void);

uint32_t syn_port_hpclock_freq_hz(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_HPCLOCK_H */
```


