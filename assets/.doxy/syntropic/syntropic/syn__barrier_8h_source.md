

# File syn\_barrier.h

[**File List**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_barrier.h**](syn__barrier_8h.md)

[Go to the documentation of this file](syn__barrier_8h.md)


```C++

#ifndef SYN_BARRIER_H
#define SYN_BARRIER_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#include "syn_compiler.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Hardware memory barrier (port function) ───────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

void syn_port_memory_barrier(void);

#endif /* SYN_USE_MULTICORE */

/* ── Acquire / Release ─────────────────────────────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

static inline void syn_store_release(volatile size_t *ptr, size_t val)
{
    SYN_COMPILER_BARRIER();
    syn_port_memory_barrier();
    *ptr = val;
}

static inline size_t syn_load_acquire(const volatile size_t *ptr)
{
    size_t val = *ptr;
    syn_port_memory_barrier();
    SYN_COMPILER_BARRIER();
    return val;
}

#else /* !SYN_USE_MULTICORE — single-core: volatile is sufficient */

static inline void syn_store_release(volatile size_t *ptr, size_t val)
{
    *ptr = val;
}

static inline size_t syn_load_acquire(const volatile size_t *ptr)
{
    return *ptr;
}

#endif /* SYN_USE_MULTICORE */

/* ── Convenience macros ────────────────────────────────────────────────── */

#define SYN_STORE_RELEASE(ptr, val)  syn_store_release((ptr), (val))

#define SYN_LOAD_ACQUIRE(ptr)        syn_load_acquire((ptr))

#ifdef __cplusplus
}
#endif

#endif /* SYN_BARRIER_H */
```


