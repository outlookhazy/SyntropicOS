

# File syn\_port\_spinlock.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spinlock.h**](syn__port__spinlock_8h.md)

[Go to the documentation of this file](syn__port__spinlock_8h.md)


```C++

#ifndef SYN_PORT_SPINLOCK_H
#define SYN_PORT_SPINLOCK_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Spinlock configuration ────────────────────────────────────────────── */

#ifndef SYN_SPINLOCK_COUNT
  #define SYN_SPINLOCK_COUNT  4
#endif

/* ── Port functions (user implements) ──────────────────────────────────── */

void syn_port_spinlock_acquire(uint8_t id);

void syn_port_spinlock_release(uint8_t id);

bool syn_port_spinlock_try_acquire(uint8_t id);

uint8_t syn_port_core_id(void);

void syn_port_ipc_notify(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MULTICORE */

#endif /* SYN_PORT_SPINLOCK_H */
```


