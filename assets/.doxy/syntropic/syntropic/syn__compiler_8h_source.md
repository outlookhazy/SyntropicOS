

# File syn\_compiler.h

[**File List**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_compiler.h**](syn__compiler_8h.md)

[Go to the documentation of this file](syn__compiler_8h.md)


```C++

#ifndef SYN_COMPILER_H
#define SYN_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Compiler detection ─────────────────────────────────────────────────── */

#if defined(__GNUC__) || defined(__clang__)
  #define SYN_COMPILER_GCC_LIKE  1
#elif defined(__ARMCC_VERSION)
  #define SYN_COMPILER_ARMCC     1
#elif defined(__IAR_SYSTEMS_ICC__)
  #define SYN_COMPILER_IAR       1
#else
  #define SYN_COMPILER_UNKNOWN   1
#endif

/* ── SYN_WEAK ──────────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_WEAK   __attribute__((weak))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_WEAK   __weak
#elif defined(SYN_COMPILER_IAR)
  #define SYN_WEAK   __weak
#else
  #define SYN_WEAK
#endif

/* ── SYN_PACKED ────────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_PACKED   __attribute__((packed))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_PACKED   __attribute__((packed))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_PACKED   /* IAR uses #pragma pack — see SYN_PRAGMA_PACK */
#else
  #define SYN_PACKED
#endif

/* ── SYN_INLINE ────────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_INLINE   static inline __attribute__((always_inline))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_INLINE   static __forceinline
#elif defined(SYN_COMPILER_IAR)
  #define SYN_INLINE   _Pragma("inline=forced") static inline
#else
  #define SYN_INLINE   static inline
#endif

/* ── SYN_UNUSED ────────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_UNUSED   __attribute__((unused))
#else
  #define SYN_UNUSED
#endif

/* ── SYN_NORETURN ──────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_NORETURN   __attribute__((noreturn))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_NORETURN   __attribute__((noreturn))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_NORETURN   __noreturn
#else
  #define SYN_NORETURN
#endif

/* ── SYN_STATIC_ASSERT ────────────────────────────────────────────────── */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  #define SYN_STATIC_ASSERT(expr, msg)   _Static_assert(expr, msg)
#else
  /* C99 fallback: negative-size array trick */
  #define SYN_STATIC_ASSERT_JOIN2(a, b) a##b
  #define SYN_STATIC_ASSERT_JOIN(a, b)  SYN_STATIC_ASSERT_JOIN2(a, b)
  #define SYN_STATIC_ASSERT(expr, msg) \
      typedef char SYN_STATIC_ASSERT_JOIN(syn_static_assert_, __LINE__)[(expr) ? 1 : -1]
#endif

/* ── SYN_SECTION ───────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_SECTION(name)   __attribute__((section(name)))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_SECTION(name)   __attribute__((section(name)))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_SECTION(name)   @ name
#else
  #define SYN_SECTION(name)
#endif

/* ── SYN_ALIGN ─────────────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_ALIGN(n)   __attribute__((aligned(n)))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_ALIGN(n)   __attribute__((aligned(n)))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_ALIGN(n)   /* IAR uses #pragma data_alignment */
#else
  #define SYN_ALIGN(n)
#endif

/* ── SYN_COMPILER_BARRIER ──────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_COMPILER_BARRIER()   __asm volatile("" ::: "memory")
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_COMPILER_BARRIER()   __asm volatile("" ::: "memory")
#elif defined(SYN_COMPILER_IAR)
  #define SYN_COMPILER_BARRIER()   __asm volatile("" ::: "memory")
#else
  #define SYN_COMPILER_BARRIER()   ((void)0)
#endif

/* ── SYN_FALLTHROUGH ───────────────────────────────────────────────────── */
#if defined(SYN_COMPILER_GCC_LIKE)
  #if __GNUC__ >= 7
    #define SYN_FALLTHROUGH   __attribute__((fallthrough))
  #else
    #define SYN_FALLTHROUGH   /* fall through */
  #endif
#else
  #define SYN_FALLTHROUGH
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_COMPILER_H */
```


