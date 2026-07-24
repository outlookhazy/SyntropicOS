

# File syn\_spinlock.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_spinlock.h**](syn__spinlock_8h.md)

[Go to the documentation of this file](syn__spinlock_8h.md)


```C++

#ifndef SYN_SPINLOCK_H
#define SYN_SPINLOCK_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../port/syn_port_spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Well-known spinlock IDs ───────────────────────────────────────────── */

#ifndef SYN_SPINLOCK_UART
  #define SYN_SPINLOCK_UART   0
#endif

#ifndef SYN_SPINLOCK_FLASH
  #define SYN_SPINLOCK_FLASH  1
#endif

#ifndef SYN_SPINLOCK_USER0
  #define SYN_SPINLOCK_USER0  2
#endif

#ifndef SYN_SPINLOCK_USER1
  #define SYN_SPINLOCK_USER1  3
#endif

/* ── Scoped lock macro ─────────────────────────────────────────────────── */

#define SYN_SPINLOCK_GUARD(id)                                     \
    for (int _sl_once = (syn_port_spinlock_acquire(id), 0);        \
         !_sl_once;                                                 \
         _sl_once = 1, syn_port_spinlock_release(id))

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_MULTICORE */

#endif /* SYN_SPINLOCK_H */
```


