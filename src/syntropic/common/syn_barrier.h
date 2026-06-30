/**
 * @file syn_barrier.h
 * @brief Memory ordering primitives for cross-core and ISR safety.
 *
 * Provides acquire/release semantics for lock-free data structures
 * that are shared between CPU cores (AMP multicore) or between ISR
 * and main context.
 *
 * @par Compile-time cost model
 *
 * When `SYN_USE_MULTICORE` is **off** (the default), the acquire/release
 * functions compile to plain volatile reads and writes — identical to
 * what the code would do without barriers. Zero overhead.
 *
 * When `SYN_USE_MULTICORE` is **on**, the functions insert hardware
 * memory barriers (via `syn_port_memory_barrier()`) at the correct
 * points to ensure cross-core visibility.
 *
 * @par Memory ordering model
 *
 * - **STORE_RELEASE:** All prior writes are visible before this store.
 *   Use after writing data, before publishing the index.
 *
 * - **LOAD_ACQUIRE:** No subsequent reads can be reordered before this
 *   load. Use after reading an index, before accessing the data it
 *   guards.
 *
 * @par Example: SPSC ring buffer with correct ordering
 * @code
 *   // Producer (core 0):
 *   buf[head] = data;
 *   SYN_STORE_RELEASE(&rb->head, next_head);
 *
 *   // Consumer (core 1):
 *   size_t h = SYN_LOAD_ACQUIRE(&rb->head);
 *   if (h != tail) {
 *       x = buf[tail];  // guaranteed to see data written before head
 *   }
 * @endcode
 * @ingroup syn_core
 */

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

/**
 * @brief Full hardware memory barrier.
 *
 * Ensures all prior loads and stores are visible to other cores/bus
 * masters before any subsequent loads or stores execute. The platform
 * port must implement this (e.g., DMB on Cortex-M, __sync_synchronize
 * as a portable fallback).
 */
void syn_port_memory_barrier(void);

#endif /* SYN_USE_MULTICORE */

/* ── Acquire / Release ─────────────────────────────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

/**
 * @brief Store with release semantics (multicore path).
 *
 * Writes @p val to the volatile size_t pointed to by @p ptr, ensuring
 * all prior writes are committed (visible to other cores) before this
 * store becomes visible.
 *
 * @param ptr  Pointer to volatile size_t.
 * @param val  Value to store.
 */
static inline void syn_store_release(volatile size_t *ptr, size_t val)
{
    SYN_COMPILER_BARRIER();
    syn_port_memory_barrier();
    *ptr = val;
}

/**
 * @brief Load with acquire semantics (multicore path).
 *
 * Reads the volatile size_t pointed to by @p ptr, ensuring no
 * subsequent loads or stores are reordered before this read.
 *
 * @param ptr  Pointer to volatile size_t.
 * @return The loaded value.
 */
static inline size_t syn_load_acquire(const volatile size_t *ptr)
{
    size_t val = *ptr;
    syn_port_memory_barrier();
    SYN_COMPILER_BARRIER();
    return val;
}

#else /* !SYN_USE_MULTICORE — single-core: volatile is sufficient */

/**
 * @brief Store with release semantics (single-core path).
 *
 * On single-core targets, volatile access is sufficient because
 * interrupts imply a full pipeline flush. This compiles to a
 * plain store — zero overhead.
 *
 * @param ptr  Pointer to volatile size_t.
 * @param val  Value to store.
 */
static inline void syn_store_release(volatile size_t *ptr, size_t val)
{
    *ptr = val;
}

/**
 * @brief Load with acquire semantics (single-core path).
 *
 * On single-core targets, volatile access is sufficient. This
 * compiles to a plain load — zero overhead.
 *
 * @param ptr  Pointer to volatile size_t.
 * @return The loaded value.
 */
static inline size_t syn_load_acquire(const volatile size_t *ptr)
{
    return *ptr;
}

#endif /* SYN_USE_MULTICORE */

/* ── Convenience macros ────────────────────────────────────────────────── */

/** @brief Store with release semantics (macro form). */
#define SYN_STORE_RELEASE(ptr, val)  syn_store_release((ptr), (val))

/** @brief Load with acquire semantics (macro form). */
#define SYN_LOAD_ACQUIRE(ptr)        syn_load_acquire((ptr))

#ifdef __cplusplus
}
#endif

#endif /* SYN_BARRIER_H */
